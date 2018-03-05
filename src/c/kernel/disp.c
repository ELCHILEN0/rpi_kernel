#include "kernel.h"

struct list_head process_list;

// Three Queues Required (Run, Turnstile, Sleep)
// Turnstile is a dependency graph...
// typedef struct {
//     spinlock_t lock;
//     struct list_head ready_queue[PRIORITY_HIGH + 1];
//     process_t *curr[NUM_CORES];
// } runqueue_t;

struct list_head ready_queue[PRIORITY_HIGH + 1];
struct list_head sleep_queue[NUM_CORES]; // Each core has its own sleep queue, avoids conflicting ticks.

process_t *running_list[NUM_CORES];

spinlock_t scheduler_lock;

/**
 * Initialize the process table and schedule lists.
 */
void disp_init() {
    for (int i = PRIORITY_IDLE; i <= PRIORITY_HIGH; i++) {
        INIT_LIST_HEAD(&ready_queue[i]);
    }

    for (int i = 0; i < NUM_CORES; i++) {
        INIT_LIST_HEAD(&sleep_queue[i]);
    }    
}

/**
 * The next function shall return the highest priority process and move the
 * process into a detatched but running state.
 */
process_t *next( void ) {
    for (int i = PRIORITY_HIGH; i >= PRIORITY_IDLE; i--) {
        __spin_lock(&scheduler_lock);
        struct list_head *head = &ready_queue[i];     

        process_t *process, *next;
        list_for_each_entry_safe(process, next, head, sched_list) {
            list_del_init(&process->sched_list);
            __spin_unlock(&scheduler_lock);

            // Reset its priority when it runs 
            // process->current_priority = process->initial_priority;
            running_list[get_core_id()] = process;

            return process;
        }

        __spin_unlock(&scheduler_lock);         
    }

    return NULL; // TODO: while(true); ... never run out of processes
}

/*
 * The ready function shall unblock a process and add it to its current
 * priority ready queue, at the end.
 */
void ready( process_t *process ) {
    process->state = RUNNABLE;
    // process->block_state = NONE;

    __spin_lock(&scheduler_lock);
    list_del_init(&process->sched_list);    
    list_add_tail(&process->sched_list, &ready_queue[process->current_priority]);
    __spin_unlock(&scheduler_lock);    
}

/*
 * The block function shall add a process to the blocked queue.  Additionally 
 * the functions block_state will be set as specified.  The process shall be 
 * removed from any existing scheduler queues.
 */
void block( process_t *process, struct list_head *queue, enum blocked_state reason ) {
    process->state = BLOCKED;
    process->block_state = reason;

    __spin_lock(&scheduler_lock);
    list_del_init(&process->sched_list);
    list_add_tail(&process->sched_list, queue);
    __spin_unlock(&scheduler_lock);        
}

// TODO: Separate into context + disp...
void common_interrupt( int interrupt_type ) {
    process_t *curr, *sched;
    curr = running_list[get_core_id()];
    sched = curr;

    // process_t *process = running_list[get_core_id()];
    switch_from(curr);

    uint64_t counter = core_timer_count();
    // if (curr->core_counter == 0) // Process hasn't run
    curr->usr_count[get_core_id()] += counter - curr->core_counter;
    curr->core_counter = counter;

    // TODO: Signal Frame
    // int next_sig = msb(process->pending_signal);
    // int curr_sig = msb(process->blocked_signal);    

    // if (next_sig != -1 && next_sig > curr_sig) {
        // aarch64_frame_t sig_frame = {
        //     .spsr = process->frame->spsr,
        //     .elr  = 0, // TODO: sigtramp
        //     .reg  = {
        //         [0 ... 31] = 0,
        //         [0] = process->frame,
        //         [1] = next_sig,
        //         [30] = 0, // TODO: sigreturn
        //     }
        // };
    //     // process->frame -= sizeof(aarch64_frame_t);
    //     // memcpy(process->frame, sig_frame, sizeof(sig_frame));

    //     process->pending_signal &= ~(1 << next_sig);
    //     process->blocked_signal |= (1 << next_sig);
    // }

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
            void *func = va_arg(args, void *);
            int stack_size = va_arg(args, int);
            code = proc_create(curr, func, stack_size, PRIORITY_MED);
            break;
        }
        case SYS_EXIT:
            code = proc_exit(curr);
            break;
        case SYS_YIELD:
            code = SCHED;
            break;
        case SYS_WAIT_PID:
        {
            pid_t pid = va_arg(args, pid_t);
            code = proc_wait(curr, pid);
            break;
        }
        case SYS_GET_PID:
            curr->ret = curr->pid;
            break;
        case SYS_KILL:
            // Later
            break;
        case SYS_SLEEP:
        {
            unsigned int ms = va_arg(args, unsigned int);
            code = proc_sleep(curr, ms);
            if (code != BLOCK) {
                // Immediate wakeup
                curr->ret = 0;
            }
            break;        
        }
        case SYS_TIME_SLICE:
            code = proc_tick(curr);
            break;

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

    counter = core_timer_count();
    curr->sys_count[get_core_id()] = counter - curr->core_counter;
    sched->core_counter = counter;

    switch_to(sched);
}