#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../sys/queue.h"
#include "../asm/atomic.h"

typedef struct sleep_queue {
    spinlock_t lock;
    TAILQ_HEAD(head, task);
} sleep_queue_t;

void sleepq_add(struct list_head *head, struct context *task, spinlock_t *lock);
void sleepq_add_locked(struct list_head *head, struct context *task);

void sleepq_alert_nr(struct list_head *head, bool (*condition)(struct context *task), unsigned int nr, spinlock_t *lock);
void sleepq_alert_locked_nr(struct list_head *head, bool (*condition)(struct context *task), unsigned int nr);

void sleepq_alert(struct list_head *head, bool (*condition)(struct context *task), spinlock_t *lock);
void sleepq_alert_locked(struct list_head *head, bool (*condition)(struct context *task));

#ifdef __cplusplus
};
#endif

#endif