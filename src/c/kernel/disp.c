#include "kernel.h"

struct list_head process_list;

// Three Queues Required (Run, Turnstile, Sleep)
// Turnstile is a dependency graph...
// typedef struct {
//     spinlock_t lock;
//     struct list_head ready_queue[PRIORITY_HIGH + 1];
//     process_t *curr[NUM_CORES];
// } runqueue_t;

#ifdef SCHED_AFFINITY
// struct list_head ready_queue[NUM_CORES][PRIORITY_HIGH + 1];

ready_queue_t ready_queue[NUM_CORES] = {0};
extern uint64_t live_procs;

#else
extern struct list_head ready_queue[PRIORITY_HIGH + 1];    
#endif

process_t *running_list[NUM_CORES];

spinlock_t scheduler_lock;

/**
 * Initialize the process table and schedule lists.
 */
void disp_init() {
    for (int i = PRIORITY_IDLE; i <= PRIORITY_HIGH; i++) {
        #ifdef SCHED_AFFINITY
        for (int j = 0; j < NUM_CORES; j++) {
            // INIT_LIST_HEAD(&ready_queue[j][i]);

            INIT_LIST_HEAD(&ready_queue[j].tasks[i]);
        }
        #else
        INIT_LIST_HEAD(&ready_queue[i]);
        #endif        
    }
}

/**
 * The next function shall return the highest priority process and move the
 * process into a detatched but running state.
 */
process_t *next( void ) {
    for (int i = PRIORITY_HIGH; i >= PRIORITY_IDLE; i--) {
        #ifdef SCHED_AFFINITY        
        // struct list_head *head = &ready_queue[get_core_id()][i];
        ready_queue_t *target_queue = &ready_queue[get_core_id()];

        __spin_lock(&target_queue->lock);
        // struct list_head *head = &ready_queue[get_core_id()].tasks[i];
        struct list_head *head = &target_queue->tasks[i];
        #else
        __spin_lock(&scheduler_lock);        
        struct list_head *head = &ready_queue[i];
        #endif     

        process_t *process, *next;
        list_for_each_entry_safe(process, next, head, sched_list) {
            list_del_init(&process->sched_list);
            
            #ifdef SCHED_AFFINITY
            target_queue->length -= 1;
            __spin_unlock(&target_queue->lock);    
            #else        
            __spin_unlock(&scheduler_lock);
            #endif

            // Reset its priority when it runs 
            // process->current_priority = process->initial_priority;
            running_list[get_core_id()] = process;

            return process;
        }

        #ifdef SCHED_AFFINITY
        __spin_unlock(&target_queue->lock);
        #else
        __spin_unlock(&scheduler_lock);   
        #endif      
    }

    return NULL; // TODO: while(true); ... never run out of processes
}

int find_busiest_core() {
    int busiest_core = 0;
    int busiest_count = 0;

    // TODO: Opportunity for other forms of "busy"
    for (int i = 0; i < NUM_CORES; i++) {
        ready_queue_t *rq = &ready_queue[i];
        if (rq->length > busiest_count) {
            busiest_core = i;
            busiest_count = rq->length;
        }
    }

    return busiest_core;
}

int find_inactive_core() {
    int inactive_core = 0;
    uint64_t inactive_count = INT_FAST64_MAX;

    // TODO: Opportunity for other forms of "busy"
    for (int i = 0; i < NUM_CORES; i++) {
        ready_queue_t *rq = &ready_queue[i];
        if (rq->length < inactive_count) {
            inactive_core = i;
            inactive_count = rq->length;
        }
    }

    return inactive_core;
}

/*
 * The ready function shall unblock a process and add it to its current
 * priority ready queue, at the end.
 */
void ready( process_t *process ) {
    process->state = RUNNABLE;
    // process->block_state = NONE;

    #ifdef SCHED_AFFINITY
    int inactive_core = find_inactive_core();
    ready_queue_t *target_queue = &ready_queue[get_core_id()];

    __spin_lock(&target_queue->lock);

    // Is it optimal to migrate this process
    if (target_queue->length * 100 / live_procs > 25) {
        __spin_unlock(&target_queue->lock);
        target_queue = &ready_queue[inactive_core];
        __spin_lock(&target_queue->lock);        
    }
    // TODO: migrating more than one processes to this queue can also be done? eg 100% with inactive core recalc

    // TODO: Possibly pull more than one process.
    
    target_queue->length += 1;
    list_del_init(&process->sched_list);
    list_add_tail(&process->sched_list, &target_queue->tasks[process->current_priority]);
    __spin_unlock(&target_queue->lock);
        
    #else
    __spin_lock(&scheduler_lock);    
    list_del_init(&process->sched_list);    
    list_add_tail(&process->sched_list, &ready_queue[process->current_priority]);
    __spin_unlock(&scheduler_lock);        
    #endif
}

// place a task on the wait queue, until it is woken with alert_on
void sleep_on(wait_queue_t *queue, process_t *task) {
    task->state = BLOCKED;

    __spin_lock(&queue->lock);

    task->curr_wait_queue = queue;
    list_del_init(&task->sched_list);
    list_add_tail(&task->sched_list, &queue->tasks);

    __spin_unlock(&queue->lock);
}

// wake up all tasks blocked on the wait queue that satisfy the condition
// TODO: wakeup a single process, regardless of the queue... (SIGNALS)
void alert_on(wait_queue_t *queue, bool (*condition)(process_t *curr)) {
    __spin_lock(&queue->lock);
    
    process_t *curr, *next;
    list_for_each_entry_safe(curr, next, &queue->tasks, sched_list) {
        if (condition(curr)) {
            list_del_init(&curr->sched_list);
            ready(curr);
        }
    }

    __spin_unlock(&queue->lock);    
}

// TODO: Separate into context + disp...
void common_interrupt( int interrupt_type ) {
    process_t *curr, *sched;
    curr = running_list[get_core_id()];
    sched = curr;

    switch_from(curr);

    for (int i = 0; i < PERF_COUNTERS; i++) {
        curr->perf_count[0][get_core_id()][i] += pmu_read_pmn(i);
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    uint64_t request, args_ptr;
    switch (interrupt_type) {
        case INT_SYSCALL:
        {
            uint64_t *params = (void *) curr->frame + sizeof(aarch64_frame_t);
            request = params[0];
            args_ptr = params[1];
            break;        
        }

        case INT_TIMER:
        {
            request = SYS_TIME_SLICE;
            break;
        }
        
        default:
            while(true); // Unhandled Interrupt 
    }

    enum syscall_return_state code = OK;

    va_list args = *(va_list *) args_ptr;
    switch (request) {
        case SYS_CREATE:
        {
            void *(*start_routine)(void *) = va_arg(args, void *);

            pthread_t discard;
            code = proc_create(curr, &discard, start_routine, NULL, PRIORITY_MED);
            break;
        }
        case SYS_EXIT:
            code = proc_exit(curr, NULL);
            break;
        case SYS_YIELD:
            code = SCHED;
            break;
        case SYS_WAIT_PID:
        {
            pid_t pid = va_arg(args, pid_t);
            code = proc_join(curr, pid, NULL);
            break;
        }
        case SYS_GET_PID:
            curr->ret = curr->pid;
            break;
        case SYS_KILL:
            // Later
            break;
        case SYS_TIME_SLICE:
            code = proc_tick(curr);
            break;

        
        case PTHREAD_CREATE:
            {
                pthread_t *thread = va_arg(args, pthread_t *);
                va_arg(args, pthread_attr_t *);
                void *(*start_routine)(void *) = va_arg(args, void *);
                void *arg = va_arg(args, void *);

                code = proc_create(curr, thread, start_routine, arg, PRIORITY_MED);
            }
            break;

        case PTHREAD_EXIT:
            {
                void *status = va_arg(args, void *);
                code = proc_exit(curr, status);
            }
            break;

        case PTHREAD_JOIN:
            {
                pthread_t thread = va_arg(args, pthread_t);
                void **status = va_arg(args, void **);

                code = proc_join(curr, thread, status);
            }
            break;

        case PTHREAD_SELF:
            code = proc_self(curr);
            break;

        case PTHREAD_MUTEX_INIT:
        case PTHREAD_MUTEX_DESTROY:
        case PTHREAD_MUTEX_LOCK:
        case PTHREAD_MUTEX_TRYLOCK:
        case PTHREAD_MUTEX_UNLOCK:
        default:
            __spin_lock(&newlib_lock);        
            printf("%-3d [core %d] dispatcher: unhandled request %ld\r\n", curr->pid, get_core_id(), request);
            __spin_unlock(&newlib_lock);            
            while(true); // Unhandled Request (trace ESR/ELR)
            break;
    }

    switch (code) {
        case OK:
            break;
        
        case SCHED:
            ready(curr);
            core_timer_rearm(TICK_REARM); // TODO: Account for time spent in syscall?                        
            // Fall through...

        case BLOCK:
        case EXIT:
            sched = next();
            break;
    }

    if (code != EXIT) {
        for (int i = 0; i < PERF_COUNTERS; i++) {
            curr->perf_count[1][get_core_id()][i] += pmu_read_pmn(i);
        }
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    switch_to(sched);
}