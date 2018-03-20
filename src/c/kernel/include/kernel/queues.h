#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "const.h"
#include "../sys/queue.h"
#include "../asm/atomic.h"

TAILQ_HEAD(task_list, context);

typedef struct ready_queue {
    spinlock_t          lock;
    int                 length;
    struct task_list    *head[PRIORITY_HIGH + 1];
} ready_queue_t;

typedef struct sleep_queue {
    spinlock_t          lock;
    struct task_list    *head;
} sleep_queue_t;

struct context;

void sleepq_add(struct task_list *head, struct context *task, spinlock_t *lock);
void sleepq_add_locked(struct task_list *head, struct context *task);

void sleepq_alert_nr(struct task_list *head, bool (*condition)(struct context *task), unsigned int nr, spinlock_t *lock);
void sleepq_alert_locked_nr(struct task_list *head, bool (*condition)(struct context *task), unsigned int nr);

void sleepq_alert(struct task_list *head, bool (*condition)(struct context *task), spinlock_t *lock);
void sleepq_alert_locked(struct task_list *head, bool (*condition)(struct context *task));

bool sleepq_condition_true(struct context *task) {
    return true;
}

#ifdef __cplusplus
};
#endif

#endif