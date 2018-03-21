#include "include/kernel/globals.h"
#include "include/kernel/context.h"

struct task_list process_list;

spinlock_t scheduler_lock;
spinlock_t newlib_lock;

uint64_t total_tasks;
uint64_t live_tasks;

struct context *running[NUM_CORES];

#ifdef SCHED_AFFINITY
ready_queue_t ready_queue[NUM_CORES] = {0};
#else
extern struct listhead *ready_queue[PRIORITY_HIGH + 1];
#endif
