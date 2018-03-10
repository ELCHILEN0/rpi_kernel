#include "kernel.h"

#ifdef SCHED_SEMAPHORE
semaphore_t sleep_sem[NUM_CORES];
semaphore_t ready_sem[NUM_CORES][PRIORITY_HIGH + 1];
#endif

void init_sema(semaphore_t *sem, int count) {
    __spin_unlock(&sem->lock);    
    INIT_LIST_HEAD(&sem->queue);    
    sem->count = count;
}

void sync_init() {
    #ifdef SCHED_SEMAPHORE
    for (int i = 0; i < NUM_CORES; i++) {
        init_sema(&sleep_sem[i], 0);

        for (int j = PRIORITY_IDLE; j <= PRIORITY_HIGH; j++) {
            init_sema(&ready_sem[i][j], 0);
        }
    }
    #endif
}

// Generic pointer to the front of a queue
struct list_head *head_pos(process_t *proc, semaphore_t *sem) {
    return &sem->queue;
}

// Generic pointer to the delta position in a queue
struct list_head *sleep_pos(process_t *proc, semaphore_t *sem) {    
    process_t *curr;
    
    // Find entry position + delta offset    
    list_for_each_entry(curr, &sem->queue, sched_list) {
        if (proc->tick_delta <= curr->tick_delta)
            break;

        proc->tick_delta -= curr->tick_delta;
    }

    if (list_empty(&sem->queue)) {
        return &sem->queue;
    } else {
        // Decrement the next entry
        list_for_each_entry_continue(curr, &sem->queue, sched_list) {
            curr->tick_delta -= proc->tick_delta;
            break;
        }

        return &curr->sched_list;
    }
}

void acquire(process_t *proc, semaphore_t *sem, struct list_head *(*find_pos)(process_t *, semaphore_t *))
{
    __spin_lock(&sem->lock);
    sem->count--;

    if (sem->count < 0) {
        list_del_init(&proc->sched_list);
        list_add_tail(&proc->sched_list, find_pos(proc, sem));
    }

    __spin_unlock(&sem->lock);
}

// void release(process_t *proc, semaphore_t *sem) {
//     __spin_lock(&sem->lock);
//     sem->count++;

//     if (sem->count <= 0) {
//         ready(proc);
//     }

//     __spin_unlock(&sem->lock);
// }

process_t *release(process_t *proc, semaphore_t *sem, struct list_head *(*find_pos)(process_t *, semaphore_t *)) {
    __spin_lock(&sem->lock);
    sem->count++;

    if (sem->count <= 0) {
        process_t *curr, *next;
        list_for_each_entry_safe(curr, next, find_pos(proc, sem), sched_list) {
            list_del_init(&curr->sched_list);
            __spin_unlock(&sem->lock);

            return curr;
        }
    }

    __spin_unlock(&sem->lock);
    return NULL;
}