#include "include/kinit.h"
#include "include/config.h"

spinlock_t newlib_lock;
sem_t psem;

// Low priority user-space process, possibly not required...
void *idle_proc(void *arg) {
    while(true) asm("wfi");

    return NULL;    
}

void *yield_proc(void *arg) {
    while(true) sched_yield();

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

// #define MATRIX_M 10
// #define MATRIX_N 20
// Small Matrix
#define MATRIX_M 20
#define MATRIX_N 25
// Average Matrix
// #define MATRIX_M 40
// #define MATRIX_N 50
// Large Matrix
// #define MATRIX_M 80
// #define MATRIX_N 100

#define STRIDE 2
#define SECTIONS STRIDE * STRIDE

// #define THREAD_POOL
#define NUM_THREADS 32
#define NUM_WORKERS 8

#define LOW_RUNTIME 100
#define MED_RUNTIME 1000
#define BIG_RUNTIME 10000

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

void *perf_scalar_multiply(void *arg) {
    for (int i = 0; i < 1000; i++) {
        scalar_multiply(samples_s[0], 2,
                        0, MATRIX_M,
                        0, MATRIX_N);
    }

    return NULL;    
}

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

typedef struct work {
    int         iters;
    int         m_start, m_end;
    int         n_start, n_end;
    uint64_t    *matrix;
    struct      list_head entry;
} work_t;

typedef struct {
    spinlock_t  lock;
    // semaphore_t sema; // compare to longer work times + longer time to insert (eg work before insert)
    bool        finished;
    struct list_head head;
    // work_t      *head;
} work_queue_t;

work_queue_t work_queue = {
    .lock = {0},
    .finished = false,
    .head = LIST_HEAD_INIT(work_queue.head),
};

void *pooled_worker(void *arg) {
    char buf[128];
    sprintf(buf, "pooled_worker - %d\r\n", *(int *) arg);
    puts(buf);

    while (true) {
        __spin_lock(&work_queue.lock);

        if (list_empty(&work_queue.head)) {
            __spin_unlock(&work_queue.lock);
            
            if (work_queue.finished)
                break;
            else {
                puts("empty\r\n");
                continue;
            }   
        }

        work_t *work = list_first_entry(&work_queue.head, work_t, entry);
        list_del(&work->entry);

        __spin_unlock(&work_queue.lock);

        for (int i = 0; i < work->iters; i++) {
            scalar_multiply(work->matrix, 2, work->m_start, work->m_end, work->n_start, work->n_end);
        }
    }

    sys_settrace(1);
    return NULL;
}

void *single_worker(void *arg) {
    puts("single_worker\r\n");
    
    work_t *work = arg;

    for (int i = 0; i < work->iters; i++) {
        scalar_multiply(work->matrix, 2, work->m_start, work->m_end, work->n_start, work->n_end);
    }

    sys_settrace(1);
    return NULL;
}

void *perf_thread_pool(void *arg) {
    puts("thread_pool\r\n");
    
    #ifdef THREAD_POOL
        pthread_t threads[NUM_WORKERS];
        for (int i = 0; i < NUM_WORKERS; i++) {
            int *arg = sys_malloc(sizeof(int));
            *arg = i;
            pthread_create(&threads[i], NULL, &pooled_worker, arg);
        }
    #else
        pthread_t threads[NUM_THREADS]; 
    #endif

    char buf[128];
    work_t *work;
    for (int i = 0; i < NUM_THREADS; i++) {
        __spin_lock(&work_queue.lock);

        work = sys_malloc(sizeof(work_t));

        work->iters = MED_RUNTIME;
        work->m_start = 0;
        work->n_start = 0;
        work->m_end = MATRIX_M;        
        work->n_end = MATRIX_N;
        work->matrix = samples_s[i % PERF_SAMPLES];        

        #ifdef THREAD_POOL
            INIT_LIST_HEAD(&work->entry);              
            list_add_tail(&work->entry, &work_queue.head);
        #else
            pthread_create(&threads[i], NULL, &single_worker, work);
        #endif

        __spin_unlock(&work_queue.lock);
    }

    work_queue.finished = true;

    #ifdef THREAD_POOL
        for (int i = 0; i < NUM_WORKERS; i++) {
            puts("waiting...\r\n");            
            pthread_join(threads[i], NULL);
        }
    #else
        for (int i = 0; i < NUM_THREADS; i++) {
            puts("waiting...\r\n");
            pthread_join(threads[i], NULL);
        }
    #endif

    puts("thread_pool - done\r\n");

    sys_settrace(1);
    return NULL;
}

void *runtime_task(void *arg) {
    int runtime = *(int *) arg;

    char buf[256];
    sprintf(buf, "runtime_task - %d\r\n", runtime);
    puts(buf);

    for (int i = 0; i < runtime; i++) {
        scalar_multiply(NULL, 2, 0, MATRIX_M, 0, MATRIX_N);
    }

    sys_settrace(1);
    return NULL;
}

void *perf_runtime(void *arg) {
    puts("runtime\r\n");

    int low = LOW_RUNTIME;
    int med = MED_RUNTIME;
    int big = BIG_RUNTIME;

    pthread_t threads[60];
    for (int i = 0; i < 20; i++) {
        pthread_create(&threads[i], NULL, runtime_task, &low);
        pthread_create(&threads[i+1], NULL, runtime_task, &med);
        pthread_create(&threads[i+2], NULL, runtime_task, &big);
    }

    for (int i = 0; i < 60; i++) {
        pthread_join(threads[i], NULL);
    }

    return NULL;
}

void *perf_death(void *arg) {
    puts("death\r\n");
    
    int low = LOW_RUNTIME;
    int big = BIG_RUNTIME;

    const int num_threads = 16 * 5;
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads / 5; i++) {
        pthread_create(&threads[i], NULL, runtime_task, &big);
        pthread_create(&threads[i+1], NULL, runtime_task, &low);
        pthread_create(&threads[i+2], NULL, runtime_task, &low);
        pthread_create(&threads[i+3], NULL, runtime_task, &low);
        pthread_create(&threads[i+4], NULL, runtime_task, &low);
    }

    for (int i = 0; i < num_threads / 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return NULL;
}

void *perf_root(void *arg) {
    size_t core_id = get_core_id();

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {
    //     pthread_t thread_id;
    //     // pthread_create(&thread_id, NULL, perf_scalar_multiply, NULL);
    //     pthread_create(&thread_id, NULL, perf_strided_scalar_multiply, NULL);
    // }

    if (core_id == 0) {
        // Thread Pool
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &perf_thread_pool, NULL);
        pthread_join(thread_id, NULL);

        // for (int i = 0; i < SECTIONS; i++) {
        //     pthread_t thread_id;
        //     // pthread_create(&thread_id, NULL, perf_scalar_multiply, NULL);
        //     pthread_create(&thread_id, NULL, &perf_strided_scalar_multiply, NULL);
        // }

        // for (int i = 0; i < SECTIONS; i++) {
        //     pthread_t thread_id;            
        //     pthread_create(&thread_id, NULL, &perf_scalar_multiply, NULL);
        // }
    }

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {    
    //     pthread_t thread_id;
    //     pthread_create(&thread_id, NULL, &perf_proc, NULL);
    // }

    // for (int i = 0; i < SECTIONS/NUM_CORES; i++) {
    //     pthread_t thread_id;        
    //     // pthread_create(&thread_id, NULL, perf_scalar_multiply, NULL);
    //     pthread_create(&thread_id, NULL, perf_strided_scalar_multiply, NULL);
    // }

    while(true) sched_yield();
}

void *blink_proc(void *arg) {
    char buf[512];

    int core_id = get_core_id();
    int core_gpio[4] = { 5, 6, 13, 19 };

    pthread_t self_id = pthread_self();    

    snprintf(buf, 512, "%-3d [core %d] blink_proc\r\n", (int) self_id, get_core_id());
    puts(buf);

    while (true) {
        int curr_core = get_core_id();
        for (int i = 0; i < 0x10000 * 3 * 30; i++);
        gpio_write(core_gpio[curr_core], true);

        for (int i = 0; i < 0x10000 * 3 * 30; i++);
        gpio_write(core_gpio[curr_core], false);
    }
}

void *sleep_proc(void *arg) {
    char buf[512];

    cpu_set_t affinity_set;
    CPU_ZERO(&affinity_set);
    CPU_SET(3, &affinity_set);
    sched_setaffinity(0, NUM_CORES, &affinity_set);

    pthread_t self_id = pthread_self();    
    
    snprintf(buf, 512, "%-3d [core %d] sleep_proc - sleep\r\n", (int) self_id, get_core_id());
    puts(buf);

    // syssleep(1 * 1000);
    for (int i = 0; i < 0x100000; i++);

    snprintf(buf, 512, "%-3d [core %d] sleep_proc - woke\r\n", (int) self_id, get_core_id());
    puts(buf);

    return NULL;    
}

void *root_proc(void *arg) {
    char buf[512];
    
    int core_id = get_core_id();
    pthread_t self_id = pthread_self();

    if (core_id == 0) {
        sem_init(&psem, 0, 1);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &sleep_proc, NULL);

        snprintf(buf, 512, "%-3d [core %d] created process with pid %d\r\n", (int) self_id, get_core_id(), (int) thread_id);
        puts(buf);

        snprintf(buf, 512, "%-3d [core %d] waiting for %d\r\n", (int) self_id, get_core_id(), (int) thread_id);
        puts(buf);

        void *join_status;
        pthread_join(thread_id, &join_status);

        snprintf(buf, 512, "%-3d [core %d] done waiting %d\r\n", (int) self_id, get_core_id(), (int) thread_id);
        puts(buf);
    } else {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &yield_proc, NULL);

        void *join_status;
        pthread_join(thread_id, &join_status);
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
    pmu_config_pmn(4, 0x14);
    pmu_config_pmn(5, 0x1);

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
    running_list[get_core_id()] = &(process_t) { };
    pthread_t idle_thread = 0;
    pthread_t root_thread = 0;

    cpu_set_t affinity;
    CPU_ZERO(&affinity);
    CPU_SET(get_core_id(), &affinity);

    proc_create(&idle_thread, &idle_proc, NULL, PRIORITY_IDLE, &affinity);
    proc_create(&root_thread, &perf_root, NULL, PRIORITY_MED, &affinity);
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
        for (int cpu = NUM_CORES - 1; cpu >= 0; cpu--) {
            core_mailbox->set[cpu][0] = true;
        }
    }

    while(true);
}


