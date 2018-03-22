#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>

#include "config.h"
#include "list.h"
#include "dispatch.h"

typedef struct cpu_set {
    uint32_t count;
} cpu_set_t;

void CPU_ZERO(cpu_set_t *set);
void CPU_SET(int cpu, cpu_set_t *set);
void CPU_CLR(int cpu, cpu_set_t *set);
int  CPU_ISSET(int cpu, cpu_set_t *set);
int  CPU_COUNT(cpu_set_t *set);

// For implementation details see the __load_context routine.
typedef struct frame {
    uint64_t spsr;    
    uint64_t elr;
    uint64_t reg[32];
} aarch64_frame_t;

typedef struct process {
    pid_t pid;
    uint64_t ret;
    uint64_t args;

    void **status;
    bool trace;

    // Scheduling
    // TODO: Idle Balancing, busiest -> idleest
    // TODO: load balance without locking? per core input channel/buffer, lazy synchronization? lockless queue or spinlock queue or try-acquire/continue?
    //
    cpu_set_t *affinityset;
    cpu_set_t affinitysetpreallocated;
    enum process_state state;
    enum blocked_state block_state;  
    enum process_priority initial_priority;
    enum process_priority current_priority;
    
    // Context + Stack
    uint64_t    stack_size;
    void        *stack_base;
    aarch64_frame_t *frame; // SP_EL0 - sizeof(aarch64_frame_t)

    // Signals (TODO)
    uint64_t    pending_signal;
    uint64_t    blocked_signal;
    // void        (*sig[32])(void *);

    // Timers + Perf Counters
    uint64_t perf_count[2][NUM_CORES][PERF_COUNTERS];
    uint64_t tick_count;
    uint64_t tick_delta;

    struct list_head process_list;
    struct list_head process_hash_list;
    struct list_head sched_list;

    struct list_head *blocked_on;
    wait_queue_t waiting;

    // Scheduling Dependencies, processes blocked on an action by this process
    // struct {
        struct list_head sending;
        struct list_head recving;
        // TODO: Can we add ready list here too, if we want to implement some sort of tree of sucessors
    // };
} process_t, pcb_t;

extern void switch_from (process_t *process);
extern void switch_to   (process_t *process);
extern void *align      (void *ptr, unsigned int alignment);

#ifdef __cplusplus
};
#endif

#endif