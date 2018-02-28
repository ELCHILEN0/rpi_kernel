#include "kernel.h"

void blink_proc() {
    int core_id = get_core_id();
    int core_gpio[4] = { 5, 6, 13, 19 };

    pid_t pid = sysgetpid();    

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] blink_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    // Blink at the rate of the original core...
    while (true) {
        int curr_core = get_core_id();
        for (int i = 0; i < 0x10000 * (2 + 1) * 30; i++);
        gpio_write(core_gpio[curr_core], true);

        for (int i = 0; i < 0x10000 * (2 + 1) * 30; i++);
        gpio_write(core_gpio[curr_core], false);
    }
}

void root_proc() {
    // while(true);
    uint8_t core_id = get_core_id();
    pid_t pid = sysgetpid();

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] root_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    syssleep(1000 * 3);

    if (core_id == 0) {
        pid_t child_pid = syscreate(blink_proc, 1024);
        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] created process with pid %d\r\n", pid, core_id, child_pid);
        __spin_unlock(&newlib_lock);
    }

    // for (int i = 0; i < 4; i++) {
    //     pid_t child_pid = syscreate(blink_proc, 1024);
    //     __spin_lock(&newlib_lock);
    //     printf("%-3d [core %d] created process with pid %d\r\n", pid, core_id, child_pid);
    //     __spin_unlock(&newlib_lock);
    // }

    while(true) sysyield();
}

// Low priority user-space process, possibly not required...
void idle_proc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    while(true) asm("wfi");
}

void timer_handler() {
    common_interrupt(INT_TIMER);
}

void svc_handler() {    
    common_interrupt(INT_SYSCALL);    
}

void kernel_release_handler() {
    __spin_lock(&newlib_lock);
    printf("[interrupt] releasing core %d\r\n", get_core_id());
    __spin_unlock(&newlib_lock); 

    core_mailbox->rd_clr[get_core_id()][0] = ~(0);         

    kernel_start();

    __spin_lock(&newlib_lock);
    printf("[kernel] unexpected execution\r\n");
    __spin_lock(&newlib_lock);
    while(true);    
}

void kernel_start() {
    uint8_t core_id = get_core_id();
    
    register_interrupt_handler(core_id, false, 4, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(core_id, false, 5, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(core_id, false, 6, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(core_id, false, 7, (interrupt_vector_t) { .handle = NULL }); 

    process_t idle_proc_stub = { };
    process_t root_proc_stub = { };
    proc_create(&idle_proc_stub, idle_proc, 4096, PRIORITY_IDLE);
    proc_create(&root_proc_stub, root_proc, 4096, PRIORITY_MED);
    if (!idle_proc_stub.ret || !root_proc_stub.ret)
        return;
  
    switch_to(next());

    // TODO: Possibly delegate this to the root_process.
    core_timer_rearm(19200000);

    asm("MSR SPSel, #0");
    asm("BL __load_context");
}

void kernel_init( void )
{
    uint8_t core_id = get_core_id();

    // Setup Dispatch Handlers
    register_interrupt_handler(core_id, false, 1, (interrupt_vector_t) { .handle = timer_handler });
    register_interrupt_handler(core_id, true, ESR_ELx_EC_SVC64, (interrupt_vector_t) { .handle = svc_handler }); 
    
    // Setup Release Handler
    register_interrupt_handler(core_id, false, 4, (interrupt_vector_t) { .handle = kernel_release_handler });
    core_mailbox_interrupt_routing(core_id, MB0_IRQ | MB1_IRQ | MB2_IRQ | MB3_IRQ);

    if (core_id == 0) {
        proc_init();
        disp_init();

        // Release Kernel Cores! (value doesnt matter)
        core_mailbox->set[3][0] = true;
        core_mailbox->set[2][0] = true;
        core_mailbox->set[1][0] = true;
        core_mailbox->set[0][0] = true;
    }

    while(true);
}


