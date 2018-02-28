#include "kernel.h"

static spinlock_t pid_lock;
static pid_t next_pid = 1;
struct list_head process_list;
struct list_head process_hash_table[PIDHASH_SZ];

void proc_init( void ) {
    // TODO: Static initialization...
    INIT_LIST_HEAD(&process_list);

    int i;
    for (i = 0; i < PIDHASH_SZ; i++) {
        INIT_LIST_HEAD(&process_hash_table[i]);
    }
}

struct list_head *get_process_bucket(pid_t pid) {
    return &process_hash_table[pid_hashfn(pid)];
}

process_t *get_process(pid_t pid) {
    process_t *p;
    struct list_head *list = get_process_bucket(pid);
    list_for_each_entry(p, list, process_hash_list) {
        if (p->pid == pid)
            return p;
    }
    
    // Ensure that references are handled apropriately here
    return NULL;
}

int create(void (*func)(), uint64_t stack_size, enum process_priority priority) {
    if (stack_size < PROC_STACK)
        stack_size = PROC_STACK;    

    __spin_lock(&newlib_lock);
    void *stack_base = malloc(stack_size);
    __spin_unlock(&newlib_lock);
    if (!stack_base)
        return 0;

    __spin_lock(&pid_lock);
    if (next_pid == 0)
        return 0;
    pid_t pid = next_pid++;
    __spin_unlock(&pid_lock);  

    process_t *process = stack_base + stack_size - sizeof(process_t);
    process->stack_base = stack_base;
    
    aarch64_frame_t frame = {
        // .spsr = 0b00000, // EL0
        .spsr = 0b00100,    // EL1t
        .elr  = (uint64_t) func,
        .reg  = {
            [0 ... 31] = 0,
            [30] = (uint64_t) sysexit
        }
    };
    process->frame = memcpy(process - sizeof(aarch64_frame_t), &frame, sizeof(frame));

    process->pid = pid;
    process->stack_size = stack_size;

    process->state = NEW;    
    process->initial_priority = priority;
    process->current_priority = priority;

    process->tick_count = 0;

    process->pending_signal = 0;
    process->blocked_signal = 0;
    
    // Process list entries
    INIT_LIST_HEAD(&process->process_list);
    INIT_LIST_HEAD(&process->process_hash_list);
    INIT_LIST_HEAD(&process->sched_list);
    // INIT_LIST_HEAD(&process->block_list);
    
    // Process Information Lists, different ways to access all processes
    // list_add(&process->process_list, &process_list);
    // list_add(&process->process_hash_list, get_process_bucket(process->pid));
   
    // Proceses waiting for interaction from me
    // INIT_LIST_HEAD(&process->waiting_list);
    // INIT_LIST_HEAD(&process->senders_list);
    // INIT_LIST_HEAD(&process->receivers_list);

    // process->sigs_pending = 0;
    // process->sigs_running = 0;

    // // TODO: memset entire process to 0
    // for (int i = SIGHUP; i < SIGSTOP; i++) {
    //     process->sig_handlers[i] = NULL;
    // }
    // process->sig_handlers[SIGSTOP] = (void*) sysstop;
    ready(process);    

    return process->pid;    
}

/**
 * The wait function will cause a processs to wait until the
 */
// int wait(process_t* process, pid_t pid) {
//     process_t* target = get_process(pid);

//     if (pid == 0 || !target || process->pid == target->pid)
//         return SYSERR;

//     list_add(&target->waiting_list, &process->block_list);
//     return BLOCKERR; 
// }

/*
 * The destroy function will unschedule the process from the dispatcher and
 * free the assocated process stack.  Additionally, every pending incoming
 * message sender will have its return code set to -1 and will be rescheduled.
 */
int destroy(process_t *process) {
    process->state = ZOMBIE;

    /*
     * Unblock processes blocked waiting to deliver a message to the current
     * process.
     */
    // list_for_each_entry_safe(p, pnext, &process->senders_list, block_list) {
    //     ready(p);
    //     p->ret = SYSERR;
    // }

    /*
     * Unblock processes blocked waiting to receive a message from the current
     * process.
     */
    // list_for_each_entry_safe(p, pnext, &process->receivers_list, block_list) {
    //     ready(p);
    //     p->ret = SYSERR;
    // }
    
    /*
     * Unblock processes waiting for the current process to die
     */
    // list_for_each_entry_safe(p, pnext, &process->waiting_list, block_list) {
    //     ready(p);
    //     p->ret = SIG_OK;
    // }

    process_t *p, *pnext;
    list_for_each_entry_safe(p, pnext, &process->blocked_waiters, blocked_on) {
        ready(p);
        if (p->blocked_cause == 0) {
            p->ret = 0;
        } else {
            p->ret = 1;
        }
    }

    list_del(&process->process_list);
    list_del(&process->process_hash_list);
    list_del(&process->sched_list);
    list_del(&process->block_list);

    free(process->stack_base);
    free(process);
    return 0;
}

/**
 * The function shall find a running process by its pid and forcefully destroy
 * it
 */
// int kill(pid_t pid) {
//     process_control_block_t *process = get_process(pid);
   
//     if (!process || process->state == TERMINATED || process->pid != pid)
//         return SYSERR;

//     return destroy(process);
// }
