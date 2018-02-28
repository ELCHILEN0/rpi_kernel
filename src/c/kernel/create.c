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

enum syscall_return_state proc_create(process_t *proc, void (*func)(), uint64_t stack_size, enum process_priority priority) {
    if (stack_size < PROC_STACK)
        stack_size = PROC_STACK;    

    __spin_lock(&newlib_lock);
    process_t *process = malloc(sizeof(process_t));
    void *stack_base = malloc(stack_size);
    __spin_unlock(&newlib_lock);
    if (!process || !stack_base) {
        // TODO: is this free of races...
        free(process);
        free(stack_base);

        proc->ret = -1;
        return OK;
    }

    pid_t pid = __atomic_fetch_add(&next_pid, 1, __ATOMIC_SEQ_CST); // TODO: __ATOMIC_RELAXED
    if (next_pid == 0) {
        proc->ret = -1;
        return OK;
    } else {
        proc->ret = pid;
    }

    aarch64_frame_t frame = {
        // .spsr = 0b00000, // EL0
        .spsr = 0b00100,    // EL1t
        .elr  = (uint64_t) func,
        .reg  = {
            [0 ... 31] = 0,
            [30] = (uint64_t) sysexit
        }
    };

    process->pid = pid;

    process->stack_base = stack_base;
    process->stack_size = stack_size;
    process->frame = memcpy(align(stack_base + stack_size - sizeof(aarch64_frame_t)), &frame, sizeof(frame));

    process->state = NEW;    
    process->initial_priority = priority;
    process->current_priority = priority;

    process->tick_count = 0;
    process->tick_delta = 0;

    // TODO: Sigs (Handlers)
    process->pending_signal = 0;
    process->blocked_signal = 0;
    
    // Process List
    INIT_LIST_HEAD(&process->process_list);
    INIT_LIST_HEAD(&process->process_hash_list);

    // Scheduling List Entries
    INIT_LIST_HEAD(&process->sched_list);
    INIT_LIST_HEAD(&process->block_list);
    
    // Scheduling List
    INIT_LIST_HEAD(&process->blocked_waiters);

    ready(process);
    return OK;    
}

enum syscall_return_state proc_wait(process_t* proc, pid_t pid) {
    process_t* process = get_process(pid);

    if (pid == 0 || !process || proc->pid == process->pid)
        return OK;

    __spin_lock(&process->blocked_waiters_lock);
    list_add(&process->blocked_waiters, &proc->block_list);
    __spin_unlock(&process->blocked_waiters_lock);    
    return BLOCK; 
}

/*
 * The destroy function will unschedule the process from the dispatcher and
 * free the assocated process stack.  Additionally, every pending incoming
 * message sender will have its return code set to -1 and will be rescheduled.
 */
enum syscall_return_state proc_exit(process_t *proc) {
    proc->state = ZOMBIE;

    process_t *p, *pnext;
    list_for_each_entry_safe(p, pnext, &proc->blocked_waiters, block_list) {
        ready(p);

        switch (p->blocked_cause) {
            case SLEEP:
                p->ret = 0;
            break;

            case WAIT:
                p->ret = 1;
            break;
        }
    }

    list_del(&proc->process_list);
    list_del(&proc->process_hash_list);
    list_del(&proc->sched_list);
    list_del(&proc->block_list);

    list_del(&proc->blocked_waiters);

    free(proc->stack_base);
    free(proc);
    return EXIT;
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
