/* disp.c : dispatcher
 */

#include <stdarg.h>
#include "kernel.h"

// int getCPUtimes(process_t *p, process_status_t *ps);

struct list_head process_list;
struct list_head ready_queues[PRIORITY_HIGH + 1];
process_t *running_list[4];
// struct list_head block_queue;
// struct list_head sleep_queue;

/* Your code goes here */
/**
 * Initialize the process table and schedule lists.
 */
void dispatcher_init() {
    for (int i = PRIORITY_IDLE; i <= PRIORITY_HIGH; i++) {
        INIT_LIST_HEAD(&ready_queues[i]);
    }

    // INIT_LIST_HEAD(&block_queue);
    // INIT_LIST_HEAD(&sleep_queue);
}

/**
 * The next function shall return the highest priority process and move the
 * process into a detatched but running state.
 */
process_t *next( void ) {
    for (int i = PRIORITY_HIGH; i >= PRIORITY_IDLE; i--) {
        struct list_head *ready_queue = &ready_queues[i];

        if (list_empty(ready_queue)) continue;

        process_t *process = list_entry(ready_queue->next, process_t, sched_list);
        // process->state = RUNNING;
        // Reset its priority when it runs 
        //process->current_priority = process->initial_priority;
        running_list[get_core_id()] = process;
        return process;
    }

    return NULL;
}

/*
 * The ready function shall unblock a process and add it to its current
 * priority ready queue, at the end.
 */
void ready( process_t *process ) {
    // process->state = READY;
    // process->block_state = NONE;

    struct list_head *ready_queue = &ready_queues[process->current_priority];

    // list_del_init(&process->block_list);
    list_del_init(&process->sched_list);
    list_add_tail(&process->sched_list, ready_queue);
}

/*
 * The block function shall add a process to the blocked queue.  Additionally 
 * the functions block_state will be set as specified.  The process shall be 
 * removed from any existing scheduler queues.
 */
// void block( process_t *process, enum blocked_state reason ) {
//     // process->state = BLOCKED;
//     // process->block_state = reason;

//     // list_del_init(&process->sched_list);
//     // list_add_tail(&process->sched_list, &block_queue);
// }

void common_interrupt( int interrupt_type ) {
    process_t *process = running_list[get_core_id()];
    switch_from(process);

    uint64_t request, args_ptr;
    switch (interrupt_type) {
        case INT_SYSCALL:
        {
            uint64_t *params = (uint64_t) process->frame + sizeof(aarch64_frame_t);
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

    va_list args = *(va_list *) args_ptr;
    switch (request) {
        case SYS_CREATE:
        {
            void *func = va_arg(args, void*);
            int stack = va_arg(args, int);
            process->ret = create(func, stack, PRIORITY_MED);
            ready(process);
            process = next();
            break;
        }
        case SYS_YIELD:
            ready(process);
            process = next();
            break;
        case SYS_WAIT_PID:
            // Fifth
            break;
        case SYS_GET_PID:
            process->ret = process->pid;
            break;
        case SYS_KILL:
            // Later
            break;
        case SYS_TIME_SLICE:
            ready(process);
            process = next();
            core_timer_rearm(19200000);
            break;

        default:
            printf("Unhandled Request %d by %d\r\n", request, process->pid);
            break;
    }

    switch_to(process);
}