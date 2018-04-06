#include "include/dispatch.h"

#include "include/kinit.h"

struct list_head process_list;

#ifdef SCHED_AFFINITY
ready_queue_t ready_queue[NUM_CORES] = {0};
#else
ready_queue_t ready_queue;
// struct list_head ready_queue[PRIORITY_HIGH + 1];    
#endif

process_t *running_list[NUM_CORES];

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
        INIT_LIST_HEAD(&ready_queue.tasks[i]);
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

        // struct list_head *head = &ready_queue[get_core_id()].tasks[i];
        #else
        ready_queue_t *target_queue = &ready_queue;
        #endif  
        __spin_lock(&target_queue->lock);           
        struct list_head *head = &target_queue->tasks[i];        

        process_t *process, *next;
        list_for_each_entry_safe(process, next, head, sched_list) {
            list_del_init(&process->sched_list);
            
            target_queue->length -= 1;            
            __spin_unlock(&target_queue->lock);                

            // Reset its priority when it runs 
            // process->current_priority = process->initial_priority;
            running_list[get_core_id()] = process;

            return process;
        }

        __spin_unlock(&target_queue->lock);
    }

    return NULL; // TODO: while(true); ... never run out of processes
}

#ifdef SCHED_AFFINITY
int sched_pickcpu_busiest() {
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

int sched_pickcpu_inactive(cpu_set_t *mask) {
    int inactive_core = 0;
    int inactive_count = INT32_MAX;

    // TODO: Opportunity for other forms of "busy"
    for (int cpu = 0; cpu < NUM_CORES; cpu++) {
        ready_queue_t *rq = &ready_queue[cpu];
        if (rq->length < inactive_count && CPU_ISSET(cpu, mask)) {
            inactive_core = cpu;
            inactive_count = rq->length;
        }
    }

    return inactive_core;
}


int sched_pickcpu(cpu_set_t *mask) {
    ready_queue_t *curr = &ready_queue[get_core_id()];

    int cpu_inactive = sched_pickcpu_inactive(mask);

    // threads with hard affinity are restricted
    // the current core is not eligble to run the process
    if (!CPU_ISSET(get_core_id(), mask)) {
        return cpu_inactive;
    }

    // threads on over-loaded cores are restricted
    // the current core is over-loaded
    if ((100 * curr->length / live_procs) > 25) {
        return cpu_inactive;
    }

    // it is not preferable to swap to a different core
    return get_core_id();
}

void sched_pull() {
    int cpu_busiest = sched_pickcpu_busiest();

    if (cpu_busiest == get_core_id())
        return;

    ready_queue_t *busy = &ready_queue[cpu_busiest];
    ready_queue_t *idle = &ready_queue[get_core_id()];

    cpu_set_t target;

    __spin_lock(&busy->lock);

    process_t *curr, *next;
    list_for_each_entry_safe_reverse(curr, next, busy->tasks, sched_list) {
        if (busy->length / idle->length < 1.2)
            break;

        CPU_ZERO(&target);
        CPU_SET(get_core_id(), &target);
        CPU_AND(&target, curr->affinityset);

        if (!CPU_ISSET(get_core_id(), &target))
            continue;
            
        busy->length--;

        cpu_set_t *restore_set = curr->affinityset;
        curr->affinityset = &target;
        ready(curr);
        curr->affinityset = restore_set;
    }

    __spin_unlock(&busy->lock);
}

#endif

/*
 * The ready function shall unblock a process and add it to its current
 * priority ready queue, at the end.
 */
void ready( process_t *process ) {
    process->state = RUNNABLE;
    // process->block_state = NONE;

    #ifdef SCHED_AFFINITY
    ready_queue_t *target_queue = &ready_queue[sched_pickcpu(process->affinityset)];
    #else
    ready_queue_t *target_queue = &ready_queue;    
    #endif

    __spin_lock(&target_queue->lock);
    target_queue->length += 1;
    list_del_init(&process->sched_list);
    list_add_tail(&process->sched_list, &target_queue->tasks[process->current_priority]);
    __spin_unlock(&target_queue->lock);
}

void sleep_on(struct list_head *head, process_t *task, spinlock_t *lock) {
    __spin_lock(lock);

    sleep_on_locked(head, task);

    __spin_unlock(lock);
}

void sleep_on_locked(struct list_head *head, process_t *task) {
    task->blocked_on = head;

    list_del_init(&task->sched_list);
    list_add_tail(&task->sched_list, head);
}

void alert_on_nr(struct list_head *head, bool (*condition)(process_t *task), unsigned int nr, spinlock_t *lock) {
    __spin_lock(lock);

    alert_on_locked_nr(head, condition, nr);

    __spin_unlock(lock);
}

void alert_on(struct list_head *head, bool (*condition)(process_t *task), spinlock_t *lock) {
    __spin_lock(lock);

    alert_on_locked(head, condition);

    __spin_unlock(lock);
}

void alert_on_locked_nr(struct list_head *head, bool (*condition)(process_t *task), unsigned int nr) {
    process_t *curr, *next;
    list_for_each_entry_safe(curr, next, head, sched_list) {
        if (nr == 0)
            break;

        if (condition(curr)) {
            list_del_init(&curr->sched_list);
            ready(curr);
        }
    }
}

void alert_on_locked(struct list_head *head, bool (*condition)(process_t *task)) {
    alert_on_locked_nr(head, condition, UINT32_MAX);
}

// TODO: Separate into context + disp...
void common_interrupt( int interrupt_type ) {
    process_t *sched;
    sched = current;

    switch_from(current);

    for (int i = 0; i < PERF_COUNTERS; i++) {
        current->perf_count[0][get_core_id()][i] += pmu_read_pmn(i);
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    uint64_t request, args_ptr;
    switch (interrupt_type) {
        case INT_SYSCALL:
        {
            uint64_t *params = (void *) current->frame + sizeof(aarch64_frame_t);
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

    enum return_state code = OK;

    va_list args = *(va_list *) args_ptr;
    switch (request) {
        case SYS_TIME_SLICE:
            {
                #ifdef SCHED_AFFINITY
                ready_queue_t *queue = &ready_queue[get_core_id()];

                queue->ticks_to_balance--;
                if (queue->ticks_to_balance == 0) {
                    queue->ticks_to_balance = CLOCK_DIVD;
                    sched_pull();
                }
                #endif
            }

            code = proc_tick();
            break;
        case SYS_PUTS:
            {
                char *str = va_arg(args, char *);
                
                __spin_lock(&newlib_lock);
                current->ret = printf("%s", str);
                __spin_unlock(&newlib_lock);
            }
            break;
        case SYS_SET_TRACE:
            {
                int trace = va_arg(args, int);
                current->trace = trace != 0;
                current->ret = 0;
            }
            break;
        case SYS_MALLOC:
            {
                size_t size = va_arg(args, size_t);
                __spin_lock(&newlib_lock);
                current->ret = malloc(size);
                __spin_unlock(&newlib_lock);
            }
            break;

        // Scheduler Routines
        case SCHED_YIELD:
            code = SCHED;
            break;
        case SCHED_SET_AFFINITY:
            {
                va_arg(args, pid_t);
                va_arg(args, size_t);
                cpu_set_t *affinityset = va_arg(args, cpu_set_t *);

                current->affinityset = affinityset;
                current->ret = 0;

                // Reschedule a process if it is no longer eligible for the current core
                if (CPU_ISSET(get_core_id(), current->affinityset))
                    code = SCHED;
            }
            break;
        case SCHED_GET_AFFINITY:
            {
                va_arg(args, pid_t);
                va_arg(args, size_t);
                cpu_set_t *mask = va_arg(args, cpu_set_t *);

                mask = current->affinityset;
                current->ret = 0;
            }
            break;

        // PThread: Basic Routines
        case PTHREAD_CREATE:
            {
                pthread_t *thread = va_arg(args, pthread_t *);
                va_arg(args, pthread_attr_t *);
                void *(*start_routine)(void *) = va_arg(args, void *);
                void *arg = va_arg(args, void *);
                cpu_set_t *affinityset = va_arg(args, cpu_set_t *);

                code = proc_create(thread, start_routine, arg, PRIORITY_MED, affinityset);
            }
            break;

        case PTHREAD_EXIT:
            {
                void *status = va_arg(args, void *);
                code = proc_exit(status);
            }
            break;

        case PTHREAD_JOIN:
            {
                pthread_t thread = va_arg(args, pthread_t);
                void **status = va_arg(args, void **);

                code = proc_join(thread, status);
            }
            break;

        case PTHREAD_SELF:
            current->ret = current->pid;
            break;

        // Semaphore: Synchronization Routines
        case SEM_INIT:
            {
                sem_t *sem = va_arg(args, sem_t *);
                int pshared = va_arg(args, int);
                unsigned int value = va_arg(args, unsigned int);

                code = __sem_init(sem, pshared, value);
            }
            break;

        case SEM_DESTROY:
            {
                sem_t *sem = va_arg(args, sem_t *);

                code = __sem_destroy(sem);
            }
            break;

        case SEM_WAIT:
            {
                sem_t *sem = va_arg(args, sem_t *);

                code = __sem_wait(sem);
            }
            break;

        case SEM_TRY_WAIT:
            {
                sem_t *sem = va_arg(args, sem_t *);

                code = __sem_trywait(sem);
            }
            break;

        case SEM_POST:
            {
                sem_t *sem = va_arg(args, sem_t *);

                code = __sem_post(sem);
            }
            break;

        case SEM_GET_VALUE:
            {
                sem_t *sem = va_arg(args, sem_t *);
                int *sval = va_arg(args, int *);

                code = __sem_getvalue(sem, sval);
            }
            break;


        // PThread: Synchronization Routines
        case PTHREAD_MUTEX_INIT:
        case PTHREAD_MUTEX_DESTROY:
        case PTHREAD_MUTEX_LOCK:
        case PTHREAD_MUTEX_TRYLOCK:
        case PTHREAD_MUTEX_UNLOCK:
        case SYS_KILL:
        case SYS_GETS:
        default:
            __spin_lock(&newlib_lock);        
            printf("%-3d [core %d] dispatcher: unhandled request %ld\r\n", current->pid, get_core_id(), request);
            __spin_unlock(&newlib_lock);            
            while(true); // Unhandled Request (trace ESR/ELR)
            break;
    }

    switch (code) {
        case OK:
            break;
        
        case SCHED:
            ready(current);
            core_timer_rearm(TICK_REARM); // TODO: Account for time spent in syscall?                        
            // Fall through...

        case BLOCK:
        case EXIT:
            sched = next();
            break;
    }

    if (code != EXIT) {
        for (int i = 0; i < PERF_COUNTERS; i++) {
            current->perf_count[1][get_core_id()][i] += pmu_read_pmn(i);
        }
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    switch_to(sched);
}