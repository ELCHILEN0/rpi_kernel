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

process_t *get_current() {
    return running_list[get_core_id()];
}

#define current get_current()

typedef struct {
    spinlock_t lock;
    struct list_head proc_list;
} run_queue_t;

typedef struct wlist_head {
    spinlock_t  lock;
    process_t   *proc;
    struct list_head proc_list;
} wait_queue_t, wlist_head_t;

// static list head...
typedef struct tlist_head {
    spinlock_t          lock;
    struct list_head    timer_list;
} tlist_head_t;

// dynamic per processes entry
typedef struct tlist_node {
    uint64_t            expiry;
    struct list_head    timer_list;
    struct list_head    proc_list;
    struct wlist_head   proc_waiters;
} tlist_node_t;

// avoids malloc between processes
// timer -> time -> time -> ... -> time
// p -|     p -|    p -|            p -|
typedef struct {
    struct tlist_head   time_list;
    struct list_head    proc_list;
} new_proc_t;

void queue_add_locked(process_t *proc, queue_t *queue) {
    list_del_init(&proc->sched_list);
    list_add_tail(&proc->sched_list, &queue->proc_list);
}


void init_wait_queue(wait_queue_t *queue) {
    *queue = (wait_queue_t) {
        .lock = 0,
        .proc = current,
        .proc_list = LIST_HEAD_INIT(queue->proc_list),
    };
}

void wait_add_locked(process_t *proc, wait_queue_t *queue) {
    list_del_init(&proc->sched_list);
    list_add_tail(&proc->sched_list, &queue->proc_list);
}

void wait_del_locked(process_t *proc) {
    list_del_init(&proc->sched_list);
    ready(proc);
}

// interrupt handlers, signals, and timeout are possible ways to be moved from the queue

#define sleep_on(queue, condition) ({ \
    bool __ret;                             \
    __spin_lock(&(queue)->lock);            \
    (queue)->proc = current;                \
    if (condition) {                        \
        __ret = false;                      \
    } else {                                \
        __ret = true;                       \
        wait_add_locked(current, queue);    \
    }                                       \
    __spin_unlock(&(queue)->lock);          \
    __ret;                                  \
})

#define wake_up(queue, condition, cmd) ({    \
    __spin_lock(&(queue)->lock);                                            \
    process_t *curr, *next;                                                 \
    list_for_each_entry_safe(curr, next, &queue->sched_list, proc_list) {   \
        queue->proc = curr;                                                 \
                                                      \
        if (condition) {                                                    \
            wait_del_locked(proc);                                          \
        }                                                                   \
    }                                                                       \
    __spin_unlock(&(queue)->lock);                                          \
})

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

// void acquire(process_t *proc, semaphore_t *sem, struct list_head *(*find_pos)(process_t *, semaphore_t *))
// {
//     __spin_lock(&sem->lock);
//     sem->count--;

//     if (sem->count < 0) {
//         list_del_init(&proc->sched_list);
//         list_add_tail(&proc->sched_list, find_pos(proc, sem));
//     }

//     __spin_unlock(&sem->lock);
// }

// void release(process_t *proc, semaphore_t *sem) {
//     __spin_lock(&sem->lock);
//     sem->count++;

//     if (sem->count <= 0) {
//         ready(proc);
//     }

//     __spin_unlock(&sem->lock);
// }

// process_t *release(process_t *proc, semaphore_t *sem, struct list_head *(*find_pos)(process_t *, semaphore_t *)) {
//     __spin_lock(&sem->lock);
//     sem->count++;

//     if (sem->count <= 0) {
//         process_t *curr, *next;
//         list_for_each_entry_safe(curr, next, find_pos(proc, sem), sched_list) {
//             list_del_init(&curr->sched_list);
//             __spin_unlock(&sem->lock);

//             return curr;
//         }
//     }

//     __spin_unlock(&sem->lock);
//     return NULL;
// }