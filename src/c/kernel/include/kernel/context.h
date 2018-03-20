#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "../sys/queue.h"
#include "sleepq.h"

typedef uint64_t pid_t;
typedef uint64_t pthread_t;

typedef struct frame {

} aarch64_frame_t;

typedef struct context {
    pid_t pid;

    // TODO: ...
    int state;
    int block_state;

    int initial_priority;
    int current_priority;

    struct frame    *frame;
    unsigned int    stack_size;
    void            *stack_base;

    TAILQ_ENTRY(task) process_list;
    TAILQ_ENTRY(task) process_hash_list;
    TAILQ_ENTRY(task) sched_list;
} process_t, thread_t, task_t;

void switch_from(struct context *context);
void switch_to  (struct context *context);

#ifdef __cplusplus
};
#endif

#endif