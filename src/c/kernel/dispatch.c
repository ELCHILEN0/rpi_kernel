#include "include/kernel.h"

#include <sys/types.h>
#include <stdarg.h>

/**
 * Initialize the process table and schedule lists.
 */
void disp_init() {
    for (int i = PRIORITY_IDLE; i <= PRIORITY_HIGH; i++) {
        #ifdef SCHED_AFFINITY
        for (int j = 0; j < NUM_CORES; j++) {
            // INIT_LIST_HEAD(&ready_queue[j][i]);


            // TAILQ_(&ready_queue[j].head[i]);
        }
        #else
        INIT_LIST_HEAD(&ready_queue[i]);
        #endif        
    }
}



// int find_busiest_core() {
//     int busiest_core = 0;
//     int busiest_count = 0;

//     // TODO: Opportunity for other forms of "busy"
//     for (int i = 0; i < NUM_CORES; i++) {
//         ready_queue_t *rq = &ready_queue[i];
//         if (rq->length > busiest_count) {
//             busiest_core = i;
//             busiest_count = rq->length;
//         }
//     }

//     return busiest_core;
// }

void common_interrupt( int interrupt_type ) {
    switch_from(current());

    for (int i = 0; i < PERF_COUNTERS; i++) {
        current()->perf_count[0][cpu_id()][i] += pmu_read_pmn(i);
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    uint64_t request, args;
    switch (interrupt_type) {
        case INT_SYSCALL:
        {
            uint64_t *params = (void *) current()->frame + sizeof(aarch64_frame_t);
            request = params[0];
            args    = params[1];
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

    dispatch_request( request, args ); // Main Dispatch Handler

    for (int i = 0; i < PERF_COUNTERS; i++) {
        current()->perf_count[1][get_core_id()][i] += pmu_read_pmn(i);
    }
    // pmu_reset_ccnt();
    pmu_reset_pmn();

    switch_to(current());
}

void dispatch_request( int request, int args_ptr ) {
    int request_ret = 0;

    va_list args = *(va_list *) args_ptr;
    switch (request) {
        case SYS_TIME_SLICE:
            // code = proc_tick();
            break;
        case SYS_PUTS:
            {
                // char *str = va_arg(args, char *);
                
                // __spin_lock(&newlib_lock);
                // current->ret = printf("%s", str);
                // __spin_unlock(&newlib_lock);
            }
            break;

        // Scheduler Routines
        case SCHED_YIELD:
            request_ret = SCHED;
            break;
        case SCHED_SET_AFFINITY:
            {
                va_arg(args, pid_t);
                va_arg(args, size_t);
                cpu_set_t *mask = va_arg(args, cpu_set_t *);
                request_ret = ksched_set_affinity(0, 0, mask);

                // Reschedule a process if it is no longer eligible for the current core
                // if (CPU_ISSET(get_core_id(), &current->affinity))
                //     code = SCHED;
            }
            break;
        case SCHED_GET_AFFINITY:
            {
                va_arg(args, pid_t);
                va_arg(args, size_t);
                cpu_set_t *mask = va_arg(args, cpu_set_t *);

                request_ret = ksched_get_affinity(0, 0, mask);
            }
            break;

        // PThread: Basic Routines
        case PTHREAD_CREATE:
            {
                pthread_t *thread = va_arg(args, pthread_t *);
                va_arg(args, pthread_attr_t *);
                void *(*start_routine)(void *) = va_arg(args, void *);
                void *arg = va_arg(args, void *);

                request_ret = proc_create(thread, start_routine, arg, PRIORITY_MED);
            }
            break;

        case PTHREAD_EXIT:
            {
                void *status = va_arg(args, void *);
                request_ret = proc_exit(status);
            }
            break;

        case PTHREAD_JOIN:
            {
                pthread_t thread = va_arg(args, pthread_t);
                void **status = va_arg(args, void **);

                request_ret = proc_join(thread, status);
            }
            break;

        case PTHREAD_SELF:
            request_ret = current()->pid;
            break;

        // Semaphore: Synchronization Routines
        case SEM_INIT:
            {
                sem_t *sem = va_arg(args, sem_t *);
                int pshared = va_arg(args, int);
                unsigned int value = va_arg(args, unsigned int);

                request_ret = ksem_init(sem, pshared, value);
            }
            break;

        case SEM_DESTROY:
            {
                sem_t *sem = va_arg(args, sem_t *);

                request_ret = ksem_destroy(sem);
            }
            break;

        case SEM_WAIT:
            {
                sem_t *sem = va_arg(args, sem_t *);

                request_ret = ksem_wait(sem);
            }
            break;

        case SEM_TRY_WAIT:
            {
                sem_t *sem = va_arg(args, sem_t *);

                request_ret = ksem_trywait(sem);
            }
            break;

        case SEM_POST:
            {
                sem_t *sem = va_arg(args, sem_t *);

                request_ret = ksem_post(sem);
            }
            break;

        case SEM_GET_VALUE:
            {
                sem_t *sem = va_arg(args, sem_t *);
                int *sval = va_arg(args, int *);

                request_ret = ksem_getvalue(sem, sval);
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
            __spin_acquire(&newlib_lock);        
            printf("%-3d [core %d] dispatcher: unhandled request %ld\r\n", current()->pid, cpu_id(), request);
            __spin_release(&newlib_lock);            
            while(true); // Unhandled Request (trace ESR/ELR)
            break;
    }

    current()->ret = request_ret;

    switch (current()->state) {
        case OK:
            break;
        
        case SCHED:
            set_runnable(current());
            core_timer_rearm(TICK_REARM); // TODO: Account for time spent in syscall?                        
            // Fall through...

        case BLOCK:
        case EXIT:
            set_current(runnable());
            break;
    }
}