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

typedef struct {
    spinlock_t  lock;
    process_t   *proc;
    struct list_head proc_list;
} wait_queue_t;

typedef struct {
    spinlock_t lock;
    struct list_head timer_list;
} timer_head_t;

typedef struct {
    spinlock_t      lock;
    uint64_t        expiry;
    wait_queue_t    waiters;
    struct list_head timer_list;    
} timer_queue_t;

void init_wait_queue(wait_queue_t *queue) {
    *queue = (wait_queue_t) {
        .lock = 0,
        .proc = current,
        .proc_list = LIST_HEAD_INIT(queue->proc_list),
    };
}

void init_timer_queue(timer_queue_t *queue) {
    *queue = (timer_queue_t) {
        .timer_list = LIST_HEAD_INIT(queue->timer_list),
    };

    init_wait_queue(&queue->waiters);
}

void wait_add_locked(process_t *proc, wait_queue_t *queue) {
    list_del_init(&proc->sched_list);
    list_add_tail(&proc->sched_list, &queue->proc_list);
}

void wait_del_locked(process_t *proc) {
    list_del_init(&proc->sched_list);
    ready(proc);
}

#define sleep_on_interruptible(queue, condition) ({ \
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

#define wake_up(queue, condition) ({    \
    __spin_lock(&(queue)->lock);                                            \
    process_t *curr, *next;                                                 \
    list_for_each_entry_safe(curr, next, &queue->sched_list, proc_list) {   \
        queue->proc = curr;                                                 \
        if (condition) {                                                    \
            wait_del_locked(proc);                                          \
        }                                                                   \
    }                                                                       \
    __spin_unlock(&(queue)->lock);                                          \
})

void timer_add(process_t *proc, timer_head_t *head, uint64_t expiry) {
    __spin_lock(&head->lock);

    if (expiry > 0) {
        timer_queue_t *curr;

        list_for_each_entry(curr, &head->timer_list, timer_list) {
            if (expiry <= curr->expiry)
                break;

            expiry -= curr->expiry;
        }

        if (list_empty(&head->timer_list)) {
            timer_queue_t *new = malloc(sizeof(timer_queue_t));
            if (!new)
                while(true);

            init_timer_queue(new);
            new->expiry = expiry;
            list_add_tail(&head->timer_list, &new->timer_list);

            bool slept = sleep_on_interruptible(&new->waiters, false);
        } else {
            
        }
    }

    __spin_unlock(&head->lock);
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