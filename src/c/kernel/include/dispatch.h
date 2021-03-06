#ifndef DISPATCH_H
#define DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "../../include/multicore.h"

typedef struct ready_queue {
    int                 ticks_to_balance;
    
    spinlock_t          lock;
    int                 length;
    struct list_head    tasks[PRIORITY_HIGH + 1];
    // TODO:
    // struct {
    //     spinlock_t lock;
    //     struct list_head tasks;
    // } plist[PRIORITY_HIGH + 1];
} ready_queue_t;

typedef struct wait_queue {
    spinlock_t          lock;
    struct list_head    tasks;
} wait_queue_t;

// Blocking
void sleep_on(struct list_head *head, process_t *task, spinlock_t *lock);
void sleep_on_locked(struct list_head *head, process_t *task);

void alert_on_nr(struct list_head *head, bool (*condition)(process_t *task), unsigned int nr, spinlock_t *lock);
void alert_on_locked_nr(struct list_head *head, bool (*condition)(process_t *task), unsigned int nr);

void alert_on(struct list_head *head, bool (*condition)(process_t *task), spinlock_t *lock);
void alert_on_locked(struct list_head *head, bool (*condition)(process_t *task));

// Dispatch and Scheduling
extern process_t *next();
extern void ready(process_t *process);
extern void common_interrupt( int interrupt_type );

#ifdef __cplusplus
};
#endif

#endif