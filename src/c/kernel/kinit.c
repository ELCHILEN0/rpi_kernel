#include "kernel.h"

extern spinlock_t print_lock;

void newproc() {
    __spin_lock(&print_lock);
    printf("[newproc] started...\r\n");
    __spin_unlock(&print_lock); 

    uint32_t pid = sysgetpid();;

    __spin_lock(&print_lock);
    printf("[newproc] pid = %d\r\n", pid);
    __spin_unlock(&print_lock);

    while(true);
}

void idleproc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    __spin_lock(&print_lock);
    printf("[idleproc] started...\r\n");
    __spin_unlock(&print_lock);

    uint32_t pid = sysgetpid();;

    __spin_lock(&print_lock);
    printf("[idleproc] pid = %d\r\n", pid);
    __spin_unlock(&print_lock); 

    pid = syscreate(newproc, 1024);    

    __spin_lock(&print_lock);
    printf("[idleproc] created %d\r\n", pid);
    __spin_unlock(&print_lock); 

    sysyield();    

    while(true) {
        asm("wfi"); 
    }
}

void kernel_release_handler() {
    core_mailbox->rd_clr[get_core_id()][0] = ~(0);         

    kernel_start();
}

void kernel_start() {
    register_interrupt_handler(0, false, 4, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(0, false, 5, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(0, false, 6, (interrupt_vector_t) { .handle = NULL }); 
    register_interrupt_handler(0, false, 7, (interrupt_vector_t) { .handle = NULL }); 

    if (create(idleproc, 4096, PRIORITY_IDLE) < 0) {
        __spin_lock(&print_lock);
        printf("failed to init idle() process\r\n");
        __spin_unlock(&print_lock);            
        return;
    }

    // if (create( &root, 1024, MED) < 0) {
    //     kprintf("failed to init root() process");
    //     return;
    // }
  
    switch_to(next());
    asm("MSR SPSel, #0");
    __load_context();

    printf("[kernel] Exiting... this should not happen\r\n");
    while(true);    
}

void kernel_init( void )
{
    uint8_t core_id = get_core_id();

    // Setup Release Handler
    register_interrupt_handler(0, false, 4, (interrupt_vector_t) { .handle = kernel_release_handler });
    core_mailbox_interrupt_routing(core_id, MB0_IRQ | MB1_IRQ | MB2_IRQ | MB3_IRQ);
    
    switch (core_id) {
        case 0:
            process_init();
            dispatcher_init();
        break;
    }

    // Release Kernel Cores!
    core_mailbox->set[3][0] = false;
    core_mailbox->set[2][0] = false;
    core_mailbox->set[1][0] = false;
    core_mailbox->set[0][0] = true;

    // Opting not to have a while guard..., interrupt routine must fire
}


