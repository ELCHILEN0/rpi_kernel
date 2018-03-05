#include "kernel.h"

// Low priority user-space process, possibly not required...
void idle_proc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    while(true) asm("wfi");
}

#define SMALL_MATRIX_M 60
#define SMALL_MATRIX_N 20//0
#define SMALL_MATRIX_P 40

// RPI 3 Specs: 16KB L1P (Instruction) and 16KB L1D(Data) and 512KB L2
// 16 KB = 2K * uint64_t (8B)
// Areas of interest:
// Cache utilization with per/core processes, process migration -> how to improve scheduling
// Splitting up the task between processors, easily parallelizable independent vs problems with communication via shared memory
#define PERF_SAMPLES 5
uint64_t samples_a[PERF_SAMPLES][SMALL_MATRIX_M][SMALL_MATRIX_N] = {
    [0 ... PERF_SAMPLES - 1][0 ... SMALL_MATRIX_M - 1][0 ... SMALL_MATRIX_N - 1] = 0xa
};
uint64_t samples_b[PERF_SAMPLES][SMALL_MATRIX_N][SMALL_MATRIX_P] = {
    [0 ... PERF_SAMPLES - 1][0 ... SMALL_MATRIX_N - 1][0 ... SMALL_MATRIX_P - 1] = 0xb
};
uint64_t samples_o[PERF_SAMPLES][SMALL_MATRIX_M][SMALL_MATRIX_P] = {
    [0 ... PERF_SAMPLES - 1][0 ... SMALL_MATRIX_M - 1][0 ... SMALL_MATRIX_P - 1] = 0    
};

void inner_multiply(uint64_t **a, uint64_t **b, uint64_t **o,   int m_start, int m_end,
                                                                int p_start, int p_end,
                                                                int n_len) {
    for (int m = m_start; m < m_end; m++) {
        for (int p = p_start; p < p_end; p++) {

            uint64_t sum = 0;

            for (int n = 0; n < n_len; n++) {
                // uint64_t tmp = a[m][n] * b[n][p];
                // __atomic_fetch_add(&o[m][p], tmp, __ATOMIC_RELAXED);
                sum += a[m][n] * b[n][p];
            }

            o[m][p] = sum;

        }
    }
}

int perf_id = 0;
void perf_proc() {
    int sample_id = __atomic_fetch_add(&perf_id, 1, __ATOMIC_RELAXED) % PERF_SAMPLES;
    __spin_lock(&newlib_lock);
    printf("(%d, %d) -> (%d, %d)\r\n", 0, 0, SMALL_MATRIX_P, SMALL_MATRIX_M);
    __spin_unlock(&newlib_lock);

    for (int rep = 0; rep < 100; rep++) {
        // Why is this failing?
        // inner_multiply(samples_a[sample_id], samples_b[sample_id], samples_o[sample_id], 
        //             0, SMALL_MATRIX_M,
        //             0, SMALL_MATRIX_P,
        //             SMALL_MATRIX_N);

        for (int m = 0; m < SMALL_MATRIX_M; m++) {
            for (int p = 0; p < SMALL_MATRIX_P; p++) {

                uint64_t sum = 0;
                for (int n = 0; n < SMALL_MATRIX_N; n++) {
                    sum += samples_a[sample_id][m][n] * samples_o[sample_id][n][p];
                }
                samples_o[sample_id][m][p] = sum;

            }
        }
                    
    }

    __spin_lock(&newlib_lock);
    printf("done %d\r\n", sample_id);
    __spin_unlock(&newlib_lock);
}

#define STRIDE ((SMALL_MATRIX_M * SMALL_MATRIX_P) / 4) // Should evenly divide

int stride_id = 0;
void perf_strided() {
    int sample_id = 4;

    int corner_id = __atomic_fetch_add(&stride_id, 1, __ATOMIC_RELAXED);    

    int x = corner_id % 2;
    int y = corner_id / 2;
    int x_div = 2; // TODO: Compute
    int y_div = 2; // TODO: Compute

    int m_start = SMALL_MATRIX_M * x/x_div;
    int p_start = SMALL_MATRIX_P * y/y_div;

    int m_end = SMALL_MATRIX_M * (x + 1)/x_div;
    int p_end = SMALL_MATRIX_P * (y + 1)/y_div;

    __spin_lock(&newlib_lock);
    printf("%d (%d, %d) -> (%d, %d)\r\n", corner_id, m_start, p_start, m_end, p_end);
    __spin_unlock(&newlib_lock);

    for (int rep = 0; rep < 100; rep++) {
        // inner_multiply(samples_a[sample_id], samples_b[sample_id], samples_o[sample_id],
        //     m_start, m_end,
        //     p_start, p_end,
        //     SMALL_MATRIX_N); 

        for (int m = m_start; m < m_end; m++) {
            for (int p = p_start; p < p_end; p++) {

                uint64_t sum = 0;
                for (int n = 0; n < SMALL_MATRIX_N; n++) {
                    sum += samples_a[sample_id][m][n] * samples_o[sample_id][n][p];
                }
                samples_o[sample_id][m][p] = sum;

            }
        }
    }

    __spin_lock(&newlib_lock);
    printf("done %d\r\n", corner_id);
    __spin_unlock(&newlib_lock);
}

void perf_root() {
    pid_t core_id = get_core_id();

    // syscreate(perf_strided, 1024);
    // Single Core:
    // syscreate(perf_strided, 1024);
    // syscreate(perf_strided, 1024);
    // syscreate(perf_strided, 1024);

    syscreate(perf_proc, 1024);
    // Single Core:
    syscreate(perf_proc, 1024);
    syscreate(perf_proc, 1024);
    syscreate(perf_proc, 1024);

    while(true) sysyield();
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

    pid_t child_pid;
    if (core_id == 0) {
        child_pid = syscreate(blink_proc, 1024);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] created process with pid %d\r\n", pid, core_id, child_pid);
        __spin_unlock(&newlib_lock);

        child_pid = syscreate(sleep_proc, 1024);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] waiting for %d\r\n", pid, core_id, child_pid);
        __spin_unlock(&newlib_lock);

        syswaitpid(child_pid);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] %d has terminated!\r\n", pid, core_id, child_pid);
        __spin_unlock(&newlib_lock);
    } else {
        syscreate(yield_proc, 1024);
        syscreate(yield_proc, 1024);
    }
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
    proc_create(&root_proc_stub, perf_root, 4096, PRIORITY_MED);
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
        // core_mailbox->set[3][0] = true;
        // core_mailbox->set[2][0] = true;
        // core_mailbox->set[1][0] = true;
        core_mailbox->set[0][0] = true;
    }

    while(true);
}


