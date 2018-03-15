#include "kernel.h"

// Low priority user-space process, possibly not required...
void *idle_proc(void *arg) {
    while(true) asm("wfi");

    return NULL;    
}

void *yield_proc(void *arg) {
    while(true) sysyield();

    return NULL;    
}

// Small Matrix
// #define MATRIX_M 10
// #define MATRIX_N 100
// #define MATRIX_P 9

// Large Matrix
// #define MATRIX_M 40
// #define MATRIX_N 100
// #define MATRIX_P 28
#define MATRIX_P 1

// Small Matrix
// #define MATRIX_M 20
// #define MATRIX_N 25
// #define MATRIX_P 1

// Average Matrix
// #define MATRIX_M 40
// #define MATRIX_N 50

// Large Matrix
#define MATRIX_M 80
#define MATRIX_N 100

/*
    Matrix Multiplication:
    ab+bc+ac = 2000
    a > 0
    0 < b < 2000/a
    0 < c < (2000 - ab)/(a + b)

    Therefore to fill up a cores l1 cache use:
    a = 10
    b = 100
    c = 9.09 ~ 9

    Therefore to fill up a cores l1 cache 4x:
    a = 40
    b = 100
    c = 28.57 ~ 29
*/

/*
    Matrix Scaling:
    ab = 2000
    a > 0
    0 < b < 2000/a

    Therefore to fill up a cores l1 cache use:
    a = 40
    b = 50

    Therefore to fill up a cores l1 cache 4x:
    a = 80
    b = 100
*/

/*
    Examples (100x each):
    SMALL_MATRIX on a SINGLE core => lower bound for thrashing
    LARGE_MATRIX on a SINGLE core => upper bound for thrashing
    SMALL_MATRIX strided on MULTIPLE cores => lower bound for multicore + thrashing
    LARGE_MATRIX strided on MULTIPLE cores => optimizeable goal => lower bound for thrashing
*/

// RPI 3 Specs: 16KB L1P (Instruction) and 16KB L1D(Data) and 512KB L2
// 16 KB = 2K * uint64_t (8B)
// Areas of interest:
// Cache utilization with per/core processes, process migration -> how to improve scheduling
// Splitting up the task between processors, easily parallelizable independent vs problems with communication via shared memory
#define PERF_SAMPLES 2
// uint64_t samples_a[PERF_SAMPLES][MATRIX_M][MATRIX_N] = {
//     [0 ... PERF_SAMPLES - 1][0 ... MATRIX_M - 1][0 ... MATRIX_N - 1] = 0xa
// };
// uint64_t samples_b[PERF_SAMPLES][MATRIX_N][MATRIX_P] = {
//     [0 ... PERF_SAMPLES - 1][0 ... MATRIX_N - 1][0 ... MATRIX_P - 1] = 0xb
// };
// uint64_t samples_o[PERF_SAMPLES][MATRIX_M][MATRIX_P] = {
//     [0 ... PERF_SAMPLES - 1][0 ... MATRIX_M - 1][0 ... MATRIX_P - 1] = 0    
// };

uint64_t samples_s[PERF_SAMPLES][MATRIX_M][MATRIX_N] = {
    [0 ... PERF_SAMPLES - 1][0 ... MATRIX_M - 1][0 ... MATRIX_N - 1] = 0xc
};

void scalar_multiply(uint64_t a[MATRIX_M][MATRIX_N],
                        const int factor,  
                        const int m_start, const int m_end,
                        const int n_start, const int n_end)
{
    // assert(m_start > 0 && n_start > 0);
    // assert(m_end <= MATRIX_M && n_end <= MATRIX_N);

    for (int m = m_start; m < m_end; m++) {
        for (int n = n_start; n < n_end; n++) {
            a[m][n] *= factor;
        }
    }
}

void inner_multiply(const uint64_t a[MATRIX_M][MATRIX_N],
                    const uint64_t b[MATRIX_N][MATRIX_P],
                    uint64_t o[MATRIX_M][MATRIX_P],
                    const int m_start, const int m_end,
                    const int p_start, const int p_end)                    
{
    // assert(m_start > 0 && p_start > 0);
    // assert(m_end <= MATRIX_M && p_end <= MATRIX_P);

    for (int m = m_start; m < m_end; m++) {
        for (int p = p_start; p < p_end; p++) {

            uint64_t sum = 0;

            for (int n = 0; n < MATRIX_N; n++) {
                // uint64_t tmp = a[m][n] * b[n][p];
                // __atomic_fetch_add(&o[m][p], tmp, __ATOMIC_RELAXED);
                sum += a[m][n] * b[n][p];
            }

            o[m][p] = sum;

        }
    }
}

#define STRIDE 2
#define SECTIONS STRIDE * STRIDE

void *perf_scalar_multiply(void *arg) {
    for (int i = 0; i < 1000; i++) {
        scalar_multiply(samples_s[0], 2,
                        0, MATRIX_M,
                        0, MATRIX_N);
    }

    return NULL;    
}

static spinlock_t my_lock;

static int scalar_multiply_id = 0;
void *perf_strided_scalar_multiply(void *arg) {
    int my_id = __atomic_fetch_add(&scalar_multiply_id, 1, __ATOMIC_RELAXED) % SECTIONS;

    const int x = my_id % STRIDE;
    const int y = my_id / STRIDE;

    const int m_start = MATRIX_M * x/STRIDE;
    const int n_start = MATRIX_N * y/STRIDE;

    const int m_end = MATRIX_M * (x + 1)/STRIDE;
    const int n_end = MATRIX_M * (y + 1)/STRIDE;

    for (int rep = 0; rep < 1000; ++rep) {
        scalar_multiply(samples_s[0], 2,
                        m_start, m_end,
                        n_start, n_end);
    }

    return NULL;    
}

void *perf_root(void *arg) {
    pid_t core_id = get_core_id();

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {
    //     // syscreate(perf_scalar_multiply, NULL);
    //     syscreate(perf_strided_scalar_multiply, NULL);
    // }

    if (core_id == 0) {
        for (int i = 0; i < SECTIONS; i++) {
            // syscreate(perf_scalar_multiply, NULL);
            syscreate(&perf_strided_scalar_multiply, NULL);
        }

        for (int i = 0; i < SECTIONS; i++) {
            syscreate(&perf_scalar_multiply, NULL);
        }
    }

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {    
    //     syscreate(perf_proc, NULL);
    // }

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {
    //     // syscreate(perf_scalar_multiply, NULL);
    //     syscreate(perf_strided_scalar_multiply, NULL);
    // }

    while(true) sysyield();
}

void *blink_proc(void *arg) {
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

void *sleep_proc(void *arg) {
    int core_id = get_core_id();
    pid_t pid = sysgetpid();    
    
    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] sleep_proc\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    // syssleep(1 * 1000);
    for (int i = 0; i < 0x100000; i++);

    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] sleep_proc - woke\r\n", pid, core_id);
    __spin_unlock(&newlib_lock);

    return NULL;    
}

void *root_proc(void *arg) {
    uint8_t core_id = get_core_id();
    pid_t pid = sysgetpid();

    if (core_id == 0) {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &sleep_proc, NULL);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] created process with pid %d\r\n", pid, core_id, thread_id);
        printf("%-3d [core %d] waiting for %d\r\n", pid, core_id, thread_id);
        __spin_unlock(&newlib_lock);

        void *join_status;
        pthread_join(thread_id, &join_status);

        __spin_lock(&newlib_lock);
        printf("%-3d [core %d] %d has terminated!\r\n", pid, core_id, thread_id);
        __spin_unlock(&newlib_lock);
    } else {
        syscreate(&yield_proc, NULL);
        syscreate(&yield_proc, NULL);
    }

    return NULL;
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
    pthread_t idle_thread = 0;
    pthread_t root_thread = 0;
    proc_create(&idle_proc_stub, &idle_thread, &idle_proc, NULL, PRIORITY_IDLE);
    proc_create(&root_proc_stub, &root_thread, &root_proc, NULL, PRIORITY_MED);
    if (!idle_thread || !root_thread)
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


