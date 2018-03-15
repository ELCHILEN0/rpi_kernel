#include "kernel.h"

static pid_t next_pid = 1;
uint64_t live_procs = 0;

spinlock_t process_list_lock;
spinlock_t process_hash_lock;
struct list_head process_list;
struct list_head process_hash[PIDHASH_SZ];

void proc_init( void ) {
    // TODO: Static initialization...
    INIT_LIST_HEAD(&process_list);

    int i;
    for (i = 0; i < PIDHASH_SZ; i++) {
        INIT_LIST_HEAD(&process_hash[i]);
    }
}

struct list_head *get_process_bucket(pid_t pid) {
    return &process_hash[pid_hashfn(pid)];
}

process_t *get_process(pid_t pid) {
    process_t *p;
    __spin_lock(&process_hash_lock);    
    list_for_each_entry(p, get_process_bucket(pid), process_hash_list) {
        if (p->pid == pid) {
            __spin_unlock(&process_hash_lock);
            return p;
        }
    }
    __spin_unlock(&process_hash_lock);
    
    // Ensure that references are handled apropriately here
    return NULL;
}

enum syscall_return_state proc_create(process_t *proc, void (*func)(), uint64_t stack_size, enum process_priority priority) {
    if (stack_size < PROC_STACK)
        stack_size = PROC_STACK;   

    __spin_lock(&newlib_lock);
    process_t *process = malloc(sizeof(process_t));
    void *stack_base = malloc(stack_size);
    // printf("process: %d, stack: %d\r\n", process, stack_base);
    __spin_unlock(&newlib_lock);
    if (!process || !stack_base) {
        // TODO: is this free of races...
        free(process);
        free(stack_base);

        proc->ret = -1;
        return OK;
    }

    pid_t pid = __atomic_fetch_add(&next_pid, 1, __ATOMIC_RELAXED); // TODO: __ATOMIC_RELAXED
    if (next_pid == 0) {
        proc->ret = -1;
        return OK;
    } else {
        proc->ret = pid;
    }

    __atomic_fetch_add(&live_procs, 1, __ATOMIC_RELAXED);

    aarch64_frame_t frame = {
        // .spsr = 0b00000, // EL0
        .spsr = 0b00100,    // EL1t
        .elr  = (uint64_t) func,
        .reg  = {
            [0 ... 31] = 0,
            [30] = (uint64_t) sysexit
        }
    };
    process->ret = frame.reg[0];

    process->pid = pid;

    process->stack_base = stack_base;
    process->stack_size = stack_size;
    process->frame = memcpy(align(stack_base + stack_size - sizeof(aarch64_frame_t)), &frame, sizeof(frame));

    process->state = NEW;    
    process->initial_priority = priority;
    process->current_priority = priority;

    process->tick_count = 0; // TODO: Remove (Deprecated)
    process->tick_delta = 0;

    memset(process->perf_count, 0, sizeof(process->perf_count));

    // TODO: Sigs (Handlers)
    process->pending_signal = 0;
    process->blocked_signal = 0;
    
    // Process List
    INIT_LIST_HEAD(&process->process_list);
    INIT_LIST_HEAD(&process->process_hash_list);

    // Scheduling List Entries
    INIT_LIST_HEAD(&process->sched_list);
    
    // Scheduling List
    INIT_LIST_HEAD(&process->waiting.tasks);
    INIT_LIST_HEAD(&process->sending);
    INIT_LIST_HEAD(&process->recving);

    __spin_lock(&process_list_lock);
    __spin_lock(&process_hash_lock);
    list_add(&process->process_list, &process_list);
    list_add(&process->process_hash_list, get_process_bucket(process->pid));
    __spin_unlock(&process_hash_lock);
    __spin_unlock(&process_list_lock);

    ready(process);
    return OK;    
}

enum syscall_return_state proc_wait(process_t* proc, pid_t pid) {
    process_t* process = get_process(pid);

    if (pid == 0 || !process || proc->pid == process->pid)
        return OK;

    sleep_on(proc, &process->waiting);
    return BLOCK; 
}

// Unschedule and return a processes resources to the kernel, any processes blocked on the
// process will be scheduled and have their return code set appropriately.
enum syscall_return_state proc_exit(process_t *proc) {
    proc->state = ZOMBIE;

    __atomic_fetch_sub(&live_procs, 1, __ATOMIC_RELAXED);

    bool wake_waiting(process_t *curr) { 
        curr->ret = 0;
        return true;
    }
    wake_up(&proc->waiting, wake_waiting);

    // TODO: Cleanup sleepers...
    __spin_lock(&newlib_lock);
    printf("%-3d [core %d] exiting\r\n", proc->pid, get_core_id());
    printf("MODE %10s %10s %10s %10s %10s %10s\r\n",    "instrs", 
                                                        "cycles", 
                                                        "l1 access", 
                                                        "l1 refill", 
                                                        "l2 access", 
                                                        "l2 refill");
    for (int i = 0; i < NUM_CORES; i++) {
        printf("USR  %10lu %10lu %10lu %10lu %10lu %10lu\r\n",  proc->perf_count[0][i][0],
                                                                proc->perf_count[0][i][1],
                                                                proc->perf_count[0][i][2],
                                                                proc->perf_count[0][i][3],
                                                                proc->perf_count[0][i][4],
                                                                proc->perf_count[0][i][5]);
    }
    for (int i = 0; i < NUM_CORES; i++) {
        printf("SYS  %10lu %10lu %10lu %10lu %10lu %10lu\r\n",  proc->perf_count[1][i][0],
                                                                proc->perf_count[1][i][1],
                                                                proc->perf_count[1][i][2],
                                                                proc->perf_count[1][i][3],
                                                                proc->perf_count[1][i][4],
                                                                proc->perf_count[1][i][5]);
    }
    __spin_unlock(&newlib_lock);

    list_del(&proc->process_list);
    list_del(&proc->process_hash_list);
    list_del(&proc->sched_list);

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
