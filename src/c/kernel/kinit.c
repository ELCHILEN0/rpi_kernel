#include "kernel.h"

// Low priority user-space process, possibly not required...
void idle_proc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    while(true) asm("wfi");
}

void yield_proc() {
    while(true) sysyield();
}

// Small Matrix
#define MATRIX_M 10
#define MATRIX_N 100
#define MATRIX_P 9

// Large Matrix
// #define MATRIX_M 40
// #define MATRIX_N 100
// #define MATRIX_P 28

// ab+bc+ac = 2000
// a > 0
// 0 < b < 2000/a
// 0 < c < (2000 - ab)/(a + b)

// The memory access are the same, no eviction will be required
// a = 10
// b = 100
// c = 9.09 ~ 9

// baseline
// test single multiply across a region 1x the size of l1, l2 cache (no thrashing)
// test strided multiply across a region 1x the size of l1, l2 cache (no thrashing)
// a = 40
// b = 100
// c = 28.57 ~ 28

// test single multiply accross a region 4x the size (thrashing)
// test strided multiply across a region 4x the size (less->no thrashing)


// RPI 3 Specs: 16KB L1P (Instruction) and 16KB L1D(Data) and 512KB L2
// 16 KB = 2K * uint64_t (8B)
// Areas of interest:
// Cache utilization with per/core processes, process migration -> how to improve scheduling
// Splitting up the task between processors, easily parallelizable independent vs problems with communication via shared memory
#define PERF_SAMPLES 5
uint64_t samples_a[PERF_SAMPLES][MATRIX_M][MATRIX_N] = {
    [0 ... PERF_SAMPLES - 1][0 ... MATRIX_M - 1][0 ... MATRIX_N - 1] = 0xa
};
uint64_t samples_b[PERF_SAMPLES][MATRIX_N][MATRIX_P] = {
    [0 ... PERF_SAMPLES - 1][0 ... MATRIX_N - 1][0 ... MATRIX_P - 1] = 0xb
};
uint64_t samples_o[PERF_SAMPLES][MATRIX_M][MATRIX_P] = {
    [0 ... PERF_SAMPLES - 1][0 ... MATRIX_M - 1][0 ... MATRIX_P - 1] = 0    
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
    // __spin_lock(&newlib_lock);
    // printf("(%d, %d) -> (%d, %d)\r\n", 0, 0, MATRIX_P, MATRIX_M);
    // __spin_unlock(&newlib_lock);

    for (int rep = 0; rep < 100; rep++) {
        // Why is this failing?
        // inner_multiply(&samples_a[sample_id], &samples_b[sample_id], &samples_o[sample_id], 
        //             0, MATRIX_M,
        //             0, MATRIX_P,
        //             MATRIX_N);

        for (int m = 0; m < MATRIX_M; m++) {
            for (int p = 0; p < MATRIX_P; p++) {

                uint64_t sum = 0;
                for (int n = 0; n < MATRIX_N; n++) {
                    sum += samples_a[sample_id][m][n] * samples_o[sample_id][n][p];
                }
                samples_o[sample_id][m][p] = sum;

            }
        }
                    
    }
}

#define STRIDE 2
#define SECTIONS STRIDE * STRIDE

int stride_id = 0;
void perf_strided() {
    int sample_id = 4;

    int corner_id = __atomic_fetch_add(&stride_id, 1, __ATOMIC_RELAXED);    

    int x = corner_id % STRIDE;
    int y = corner_id / STRIDE;

    int m_start = MATRIX_M * x/STRIDE;
    int p_start = MATRIX_P * y/STRIDE;

    int m_end = MATRIX_M * (x + 1)/STRIDE;
    int p_end = MATRIX_P * (y + 1)/STRIDE;

    // __spin_lock(&newlib_lock);
    // printf("%d (%d, %d) -> (%d, %d)\r\n", corner_id, m_start, p_start, m_end, p_end);
    // __spin_unlock(&newlib_lock);

    for (int rep = 0; rep < 100; rep++) {
        // inner_multiply(&samples_a[sample_id], &samples_b[sample_id], &samples_o[sample_id],
        //     m_start, m_end,
        //     p_start, p_end,
        //     MATRIX_N); 

        for (int m = m_start; m < m_end; m++) {
            for (int p = p_start; p < p_end; p++) {

                uint64_t sum = 0;
                for (int n = 0; n < MATRIX_N; n++) {
                    sum += samples_a[sample_id][m][n] * samples_o[sample_id][n][p];
                }
                samples_o[sample_id][m][p] = sum;

            }
        }
    }
}

void perf_root() {
    pid_t core_id = get_core_id();

    // int p = SECTIONS/NUM_CORES;
    int p = 4; // Single Core
    // for (int i = 0; i < p; i++) {
    //     syscreate(perf_strided, 1024);
    // }

    syscreate(yield_proc, 1024);
    for (int i = 0; i < p; i++) {    
        syscreate(perf_proc, 1024);
    }
    // Single Core:
    // syscreate(perf_proc, 1024);
    // syscreate(perf_proc, 1024);
    // syscreate(perf_proc, 1024);

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

    // Setup Performance Monitor Unit
    pmu_enable();
    pmu_config_pmn(0, 0x8);
    pmu_config_pmn(1, 0x11);
    pmu_config_pmn(2, 0x4);
    pmu_config_pmn(3, 0x3);
    pmu_config_pmn(4, 0x16);
    pmu_config_pmn(5, 0x17);

    pmu_enable_pmn(0);
    pmu_enable_pmn(1);
    pmu_enable_pmn(2);
    pmu_enable_pmn(3);
    pmu_enable_pmn(4);
    pmu_enable_pmn(5);

    pmu_enable_ccnt();

    pmu_reset_ccnt();
    pmu_reset_pmn();

    // Create initial processes
    process_t idle_proc_stub = { };
    process_t root_proc_stub = { };
    proc_create(&idle_proc_stub, idle_proc, 4096, PRIORITY_IDLE);
    proc_create(&root_proc_stub, perf_root, 4096, PRIORITY_MED);
    if (!idle_proc_stub.ret || !root_proc_stub.ret)
        return;
  
    switch_to(next());

    // Setup Timer
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


