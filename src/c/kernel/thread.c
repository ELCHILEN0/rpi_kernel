#include "include/kernel.h"

void proc_init( void ) {
    // TODO: Static initialization...
    // INIT_LIST_HEAD(&process_list);
    TAILQ_INIT(&process_list);

    int i;
    for (i = 0; i < PIDHASH_SZ; i++) {
        // INIT_LIST_HEAD(&process_hash[i]);
        TAILQ_INIT(&process_hash[i]);
    }
}

struct task_list *get_process_bucket(pid_t pid) {
    return &process_hash[pid_hashfn(pid)];
}

process_t *get_process(pid_t pid) {
    process_t *p;
    __spin_acquire(&process_hash_lock);    
    TAILQ_FOREACH(p, get_process_bucket(pid), process_hash_list) {
        if (p->pid == pid) {
            __spin_release(&process_hash_lock);
            return p;
        }
    }
    __spin_release(&process_hash_lock);
    
    // Ensure that references are handled apropriately here
    return NULL;
}

int kpthread_create(pthread_t *thread, void *(*start_routine)(void *), void *arg, enum process_priority priority) {
    int stack_size = PROC_STACK;
    
    __spin_acquire(&newlib_lock);
    process_t *process = malloc(sizeof(process_t));
    void *stack_base = malloc(stack_size);
    __spin_release(&newlib_lock);
    if (!process || !stack_base) {
        // TODO: is this free of races...
        free(process);
        free(stack_base);

        return EAGAIN;
    }

    pid_t pid = __atomic_fetch_add(&total_tasks, 1, __ATOMIC_RELAXED); // TODO: __ATOMIC_RELAXED
    if (total_tasks == 0) {
        return EAGAIN;
    } else {
        *thread = pid;
    }

    __atomic_fetch_add(&live_tasks, 1, __ATOMIC_RELAXED);

    aarch64_frame_t frame = {
        // .spsr = 0b00000, // EL0
        .spsr = 0b00100,    // EL1t
        .elr  = (uint64_t) start_routine,
        .reg  = {
            [0 ... 31] = 0,
            [0] = (uint64_t) arg,
            [30] = (uint64_t) pthread_exit
        }
    };
    process->ret = frame.reg[0];

    process->pid = pid;

    process->stack_base = stack_base;
    process->stack_size = stack_size;
    process->frame = memcpy(align(stack_base + stack_size - sizeof(aarch64_frame_t), 16), &frame, sizeof(frame));

    for (int cpu = 0; cpu < NUM_CORES; cpu++) {
        CPU_SET(cpu, &process->affinity);
    }
    process->state = NEW;    
    process->initial_priority = priority;
    process->current_priority = priority;

    memset(process->perf_count, 0, sizeof(process->perf_count));
    
    // // Process List
    // INIT_LIST_HEAD(&process->process_list);
    // INIT_LIST_HEAD(&process->process_hash_list);

    // // Scheduling List Entries
    // INIT_LIST_HEAD(&process->sched_list);
    
    // Scheduling List
    __spin_acquire(&process_list_lock);
    __spin_acquire(&process_hash_lock);
    list_add(&process->process_list, &process_list);
    list_add(&process->process_hash_list, get_process_bucket(process->pid));
    __spin_release(&process_hash_lock);
    __spin_release(&process_list_lock);

    set_runnable(process);
    return 0;    
}

int kpthread_join(pthread_t thread, void **status) {
    process_t* process = get_process(thread);

    if (thread == 0 || !process || current()->pid == process->pid) {
        current()->state = BLOCK;
        return EINVAL;
    }

    current()->exit_status = status;

    sleepq_add(&process->waiting.head, current(), &process->waiting.lock);
    return BLOCK; 
}

// Unschedule and return a processes resources to the kernel, any processes blocked on the
// process will be scheduled and have their return code set appropriately.
void kpthread_exit(void *status) {
    current()->state = ZOMBIE;

    __atomic_fetch_sub(&live_tasks, 1, __ATOMIC_RELAXED);

    bool wake_waiting(process_t *curr) {
        curr->ret = 0;

        if (curr->exit_status)
            curr->exit_status = status;
        return true;
    }
    sleepq_alert(&current()->waiting.head, wake_waiting, &current()->waiting.lock);

    // TODO: Cleanup sleepers...
    __spin_acquire(&newlib_lock);
    printf("%-3d [core %d] exiting\r\n", current()->pid, cpu_id());
    printf("MODE %10s %10s %10s %10s %10s %10s\r\n",   
            "instrs", 
            "cycles", 
            "l1 access", 
            "l1 refill", 
            "l2 access", 
            "l2 refill");
    for (int i = 0; i < NUM_CORES; i++) {
        printf("USR  %10lu %10lu %10lu %10lu %10lu %10lu\r\n",  
                current()->perf_count[0][i][0],
                current()->perf_count[0][i][1],
                current()->perf_count[0][i][2],
                current()->perf_count[0][i][3],
                current()->perf_count[0][i][4],
                current()->perf_count[0][i][5]);
    }
    for (int i = 0; i < NUM_CORES; i++) {
        printf("SYS  %10lu %10lu %10lu %10lu %10lu %10lu\r\n",  
                current()->perf_count[0][i][0],
                current()->perf_count[1][i][1],
                current()->perf_count[1][i][2],
                current()->perf_count[1][i][3],
                current()->perf_count[1][i][4],
                current()->perf_count[1][i][5]);
    }
    __spin_release(&newlib_lock);

    current()->state = EXIT;
    // TAILQ_REMOVE(&process_list, current(), process_list);
    // // TAILQ_REMOVE(get_process_bucket(current->pid), current(), process_hash_list);
    // // list_del(&current()->process_list);
    // // list_del(&current()->process_hash_list);
    // // list_del(&current()->sched_list);

    // free(current()->stack_base);
    // free(current());
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
