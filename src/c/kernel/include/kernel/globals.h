#include "../asm/atomic.h"

#include "const.h"
#include "sched.h"
#include "queues.h"

spinlock_t newlib_lock;
spinlock_t scheduler_lock;
spinlock_t process_list_lock;
spinlock_t process_hash_lock;

uint64_t total_tasks;
uint64_t live_tasks;

struct task_list process_list;
struct task_list process_hash[PIDHASH_SZ];

struct context *running[NUM_CORES];

#ifdef SCHED_AFFINITY
ready_queue_t ready_queue[NUM_CORES] = {0};
#else
extern struct listhead *ready_queue[PRIORITY_HIGH + 1];
#endif
