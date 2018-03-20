#include "include/kernel/globals.h"

struct list_head process_list;

#ifdef SCHED_AFFINITY
ready_queue_t ready_queue[NUM_CORES] = {0};
#else
extern struct list_head ready_queue[PRIORITY_HIGH + 1];    
#endif

process_t *running_list[NUM_CORES];

spinlock_t scheduler_lock;
