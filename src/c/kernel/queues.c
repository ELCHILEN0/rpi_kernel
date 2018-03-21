#include "include/kernel/queues.h"
#include "include/kernel/context.h"
#include "include/kernel/sched.h"

void sleepq_add(struct task_list *head, task_t *task, spinlock_t *lock) {
    __spin_acquire(lock);

    sleepq_add_locked(head, task);

    __spin_release(lock);
}

void sleepq_add_locked(struct task_list *head, task_t *task) {
    task->blocked_on = head;

    TAILQ_REMOVE        (head, task, sched_list);
    TAILQ_INSERT_TAIL   (head, task, sched_list);
}

void sleepq_alert_nr(struct task_list *head, bool (*condition)(struct context *task), unsigned int nr, spinlock_t *lock) {
    __spin_acquire(lock);

    sleepq_alert_locked_nr(head, condition, nr);

    __spin_release(lock);
}

void sleepq_alert(struct task_list *head, bool (*condition)(task_t *task), spinlock_t *lock) {
    __spin_acquire(lock);

    sleepq_alert_locked(head, condition);

    __spin_release(lock);
}

void sleepq_alert_locked_nr(struct task_list *head, bool (*condition)(task_t *task), unsigned int nr) {
    task_t *curr, *next;

    TAILQ_FOREACH_SAFE(curr, head, sched_list, next) {
        if (nr == 0)
            break;

        if (condition(curr)) {
            TAILQ_REMOVE(head, curr, sched_list);
            set_runnable(curr);
        }
    }
}

void sleepq_alert_locked(struct task_list *head, bool (*condition)(task_t *task)) {
    sleepq_alert_locked_nr(head, condition, UINT32_MAX);
}