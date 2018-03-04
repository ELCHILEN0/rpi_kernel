#include "kernel.h"

// Low priority user-space process, possibly not required...
void idle_proc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    while(true) asm("wfi");
}

// TODO: (N*M) x (M*P) matrix sizes...
#define SMALL_MATRIX_X 1024
#define SMALL_MATRIX_Y 1024
#define SMALL_MATRIX_Z 1024
typedef uint64_t small_matrix_t[SMALL_MATRIX_X][SMALL_MATRIX_Y][SMALL_MATRIX_Z];
// uint64_t perf_segment[10][1024][1024][1024];
// uint64_t perf_
small_matrix_t samples[10][2];
small_matrix_t output[10];

// RPI 3 Specs: 16KB L1P (Instruction) and 16KB L1D(Data) and 512KB L2
int perf_id = 0;
void perf_proc() {
    int perf_id = __atomic_fetch_add(&perf_id, 1, __ATOMIC_RELAXED) % 10;

    // small_matrix_t sample[2] = samples[perf_id];
    // uint64_t mat0[][][] = samples[perf_id][0];

    /*
    l=5;m=2;n=3;p=4;q=6;
    A=randn(l,m,n);
    B=randn(n,p,q);
    C=zeros(l,m,p,q);

    for h = 1:l
        for i = 1:m
            for j = 1:p
                for g = 1:q
                    for k = 1:n
                        C(h,i,j,g) = C(h,i,j,g) + A(h,i,k)*B(k,j,g);
                    end
                end
            end
        end
    end
    */
    for (int l = 0; l < SMALL_MATRIX_X; l++) {
        for (int m = 0; m < SMALL_MATRIX_Y; m++) {
            uint64_t sum = 0;
            for (int k = 0; k < SMALL_MATRIX_Z; i++) {
                // TODO: Fix, doesnt really matter though...
                sum += samples[perf_id][0][i][j][k] * samples[perf_id][0][i][j][k];
                output[perf_id][i][j][k] = sum;
            }
        }
    }

    // TODO: Incorporate syscalls, send/recv/wait ? splitting up task with communication overheads...


}

void yield_proc() {
    while(true) sysyield();
}

void blink_proc() {
    int core_id = get_core_id();
    int core_gpio[4] = { 5, 6, 13, 19 };

    pid_t pid = sysgetpid();    

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] blink_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    while (true) {
        int curr_core = get_core_id();
        for (int i = 0; i < 0x10000 * 3 * 30; i++);
        gpio_write(core_gpio[curr_core], true);

        for (int i = 0; i < 0x10000 * 3 * 30; i++);
        gpio_write(core_gpio[curr_core], false);
    }
}

void sleep_proc() {
    int core_id = get_core_id();
    pid_t pid = sysgetpid();    
    
    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] sleep_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    syssleep(1 * 1000);

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] sleep_proc - woke\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);
}

void root_proc() {
    uint8_t core_id = get_core_id();
    pid_t pid = sysgetpid();

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] root_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    if (core_id == 0) {
        pid_t child_pid = syscreate(blink_proc, 1024);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] created process with pid %d\r\n", pid, core_id, child_pid);
        __spin_unlock(&newlib_lock);
    } else {
        syscreate(yield_proc, 512);
        syscreate(yield_proc, 512);
    }

    pid_t child_pid = syscreate(sleep_proc, 512);

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] waiting for %d\r\n", pid, core_id, child_pid);
    __spin_unlock(&newlib_lock);

    syswaitpid(child_pid);

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] %d has terminated!\r\n", pid, core_id, child_pid);
    __spin_unlock(&newlib_lock);
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

extern void __disable_interrupts(void);

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

    __disable_interrupts();
    core_timer_rearm(TICK_REARM);

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


