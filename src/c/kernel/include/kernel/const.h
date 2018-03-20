#ifndef CONST_H
#define CONST_H

#ifdef __cplusplus
extern "C" {
#endif

// begin    - feature flags
#define SCHED_AFFINITY
// end      - feature flags

#define NUM_CORES       4
#define PERF_COUNTERS   6

// Clock Frequencey 19.2 MHz
// ~ a time slice of 1 S    = 19.2e6
// ~ a time slice of 1 MS   = 19.2e6/1e3
#define CLOCK_FREQ      19200000
#define CLOCK_DIVD      10
#define TICK_REARM      (CLOCK_FREQ / CLOCK_DIVD)

// Hash Function from: http://www.tldp.org/LDP/lki/lki-2.html
#define PIDHASH_SZ      (4096 >> 6)
#define pid_hashfn(x)   ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

#define PROC_STACK      (4096 * 4)

enum sched_state {
    OK,
    SCHED,
    BLOCK,
    EXIT,
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

#ifdef __cplusplus
};
#endif

#endif