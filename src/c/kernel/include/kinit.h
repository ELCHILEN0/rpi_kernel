#ifndef KERNEL_H
#define KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include "context.h"
#include "dispatch.h" // TODO: Break Up
#include "semaphore.h"

#include "../../include/gpio.h"
#include "../../include/perf.h"
#include "../../include/timer.h"
#include "../../include/mailbox.h"
#include "../../include/multicore.h"
#include "../../include/interrupts.h"

extern spinlock_t newlib_lock;
extern spinlock_t scheduler_lock;
extern spinlock_t process_list_lock;
extern spinlock_t process_hash_lock;

extern pid_t    next_pid;
extern uint64_t live_procs;

#ifdef SCHED_AFFINITY
extern ready_queue_t ready_queue[NUM_CORES];
#else
extern spinlock_t scheduler_lock;
extern struct list_head ready_queue[];    
#endif

extern process_t *running_list[NUM_CORES];

// Initialization
extern void kernel_init();
extern void kernel_start();
extern void kernel_release_handler();

extern void proc_init();
extern void disp_init();

// Non-POSIX System Calls
extern void *sys_malloc(size_t size);

// System Call Helper Functions
extern enum return_state proc_create(pthread_t *thread, void *(*start_routine)(void *), void *arg, enum process_priority priority, cpu_set_t *affintiyset);
extern enum return_state proc_exit  (void *status);
extern enum return_state proc_join  (pid_t pid, void **status);

extern enum return_state proc_tick  ();
extern enum return_state proc_sleep (unsigned int ms);

int msb(uint64_t x);
int lsb(uint64_t x);

#ifdef __cplusplus
};
#endif

#endif