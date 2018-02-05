/* create.c : create a process
 */

#include "kernel.h"

static pid_t next_pid = 0;
struct list_head process_list;
struct list_head process_hash_table[PIDHASH_SZ];

void process_init( void ) {
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

int create(void (*func)(), int stack_size, enum process_priority priority) {
    if (stack_size < PROC_STACK)
        stack_size = PROC_STACK;    

    // Ran out of pid_t to allocate
    if (next_pid == 0 && !list_empty(&process_list))
        return -1; 

    void *stack_pointer = malloc(stack_size); // TODO: Unsafe...
    if (!stack_pointer)
        return -1;

    process_t *process = stack_pointer + stack_size - sizeof(process_t);
    process->stack_base = stack_pointer;
    process->frame = (arm_frame32_t *) (process - sizeof(arm_frame32_t) - 1 * sizeof(long));
    
    // TODO: Create with args...
    for (int i = 0; i < 13; i++) {
        process->frame->reg[i] = i * 10;
    }

    // process->frame->sp = (uint32_t) &process->frame->lr;
    process->frame->lr = (uint32_t) func;
    // process->frame->pc = (uint32_t) NULL;
    // asm("MRS %0, CPSR" :: "r" (process->frame->cpsr));
    
    //process->state = READY;
    process->pid = next_pid++;
    process->stack_size = stack_size;
    process->initial_priority = priority;
    process->current_priority = priority;
    
    // int i = context_switch(process);

    // Process list entries
    INIT_LIST_HEAD(&process->process_list);
    INIT_LIST_HEAD(&process->process_hash_list);
    INIT_LIST_HEAD(&process->sched_list);
    // INIT_LIST_HEAD(&process->block_list);
    
    // Process Information Lists, different ways to access all processes
    list_add(&process->process_list, &process_list);
    list_add(&process->process_hash_list, get_process_bucket(process->pid));
   
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
// int destroy(process_control_block_t *process) {
//     process->state = TERMINATED;
//     process_t *p, *pnext;

//     /*
//      * Unblock processes blocked waiting to deliver a message to the current
//      * process.
//      */
//     list_for_each_entry_safe(p, pnext, &process->senders_list, block_list) {
//         ready(p);
//         p->ret = SYSERR;
//     }

//     /*
//      * Unblock processes blocked waiting to receive a message from the current
//      * process.
//      */
//     list_for_each_entry_safe(p, pnext, &process->receivers_list, block_list) {
//         ready(p);
//         p->ret = SYSERR;
//     }
    
//     /*
//      * Unblock processes waiting for the current process to die
//      */
//     list_for_each_entry_safe(p, pnext, &process->waiting_list, block_list) {
//         ready(p);
//         p->ret = SIG_OK;
//     }

//     list_del(&process->process_list);
//     list_del(&process->process_hash_list);
//     list_del(&process->sched_list);
//     list_del(&process->block_list);

//     return kfree(process->stack_pointer);
// }

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
