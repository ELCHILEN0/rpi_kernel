#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "../sys/queue.h"

#include "const.h"
#include "sched.h"
#include "queues.h"

// typedef uint64_t pid_t;
// typedef uint64_t pthread_t;

typedef struct frame {
    uint64_t spsr;    
    uint64_t elr;
    uint64_t reg[32];
} aarch64_frame_t;

typedef struct context {
    pid_t pid;
    uint64_t ret;

    void (*fun)(void *);
    void *arg;
    void *exit_status;

    // Identification Parameters
    int state;
    int block_state;

    // Scheduling Parameters
    cpu_set_t affinity;
    int initial_priority;
    int current_priority;

    // Context Runtime
    struct frame    *frame;
    unsigned int    stack_size;
    void            *stack_base;

    uint64_t perf_count[2][NUM_CORES][PERF_COUNTERS];    

    TAILQ_ENTRY(context) process_list;
    TAILQ_ENTRY(context) process_hash_list;
    TAILQ_ENTRY(context) sched_list;

    struct sleep_queue waiting;

    struct task_list *blocked_on;
} process_t, thread_t, task_t;

void switch_from(struct context *context);
void switch_to  (struct context *context);

#ifdef __cplusplus
};
#endif

#endif