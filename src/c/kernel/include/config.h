#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include <pthread.h>
#include <sched.h>

#include "list.h"

#define SCHED_AFFINITY

// #define NUM_CORES       4
#define NUM_CORES       1
#define PERF_COUNTERS   6

// Clock Frequencey 19.2 MHz
// ~ a time slice of 1 S = 19.2e6
// ~ a time slice of 1 MS = 19.2e6/1e3
#define CLOCK_FREQ      19200000
#define CLOCK_DIVD      10
#define TICK_REARM      (CLOCK_FREQ / CLOCK_DIVD)

// Hash Function from: http://www.tldp.org/LDP/lki/lki-2.html
#define PIDHASH_SZ      (4096 >> 6)
#define pid_hashfn(x)   ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

#define PROC_STACK      (4096 * 4)

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define current         running_list[get_core_id()]

enum return_state {
    OK,
    SCHED,
    BLOCK,
    EXIT,
};

enum interrupt_request {
    SYS_TIME_SLICE,

    SYS_KILL,
    SYS_SLEEP,
    SYS_PUTS,
    SYS_GETS,  
    SYS_SET_TRACE,
    SYS_GET_TRACE, 
    SYS_MALLOC,

    SCHED_YIELD,
    SCHED_SET_AFFINITY,
    SCHED_GET_AFFINITY,

    PTHREAD_CREATE,
    PTHREAD_EQUAL,
    PTHREAD_EXIT,
    PTHREAD_JOIN,
    PTHREAD_SELF,
    PTHREAD_MUTEX_INIT,
    PTHREAD_MUTEX_DESTROY,
    PTHREAD_MUTEX_LOCK,
    PTHREAD_MUTEX_TRYLOCK,
    PTHREAD_MUTEX_UNLOCK,

    SEM_INIT,
    SEM_DESTROY,
    SEM_WAIT,
    SEM_TRY_WAIT,
    SEM_POST,
    SEM_GET_VALUE,
};

enum interrupt_source {
    INT_SYSCALL,
    INT_TIMER,
};

enum blocked_state {
    WAIT,
    // SLEEP,
    // SEND,
    // RECV,
    // READ,
    // WRITE,
};

enum process_priority {
    PRIORITY_IDLE,
    PRIORITY_LOW,
    PRIORITY_MED,
    PRIORITY_HIGH,
};

enum process_state {
    NEW,
    RUNNABLE,
    BLOCKED,
    STOPPED,
    ZOMBIE,
};

typedef struct frame aarch64_frame_t;
typedef struct process process_t;

#ifdef __cplusplus
};
#endif

#endif