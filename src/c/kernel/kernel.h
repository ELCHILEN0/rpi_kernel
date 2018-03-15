#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>
#include <string.h>

#include "list.h"

#include "../gpio.h"
#include "../perf.h"
#include "../timer.h"
#include "../mailbox.h"
#include "../multicore.h"
#include "../interrupts.h"

#define SCHED_AFFINITY

#define NUM_CORES 4
#define PERF_COUNTERS 6

// Clock Frequencey 19.2 MHz
// ~ a time slice of 1 S = 19.2e6
// ~ a time slice of 1 MS = 19.2e6/1e3
#define CLOCK_FREQ 19200000
#define CLOCK_DIVD 10
#define TICK_REARM (CLOCK_FREQ / CLOCK_DIVD)

// Hash Function from: http://www.tldp.org/LDP/lki/lki-2.html
#define PIDHASH_SZ (4096 >> 6)
#define pid_hashfn(x)   ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

#define PROC_STACK (4096 * 4)

extern uint64_t live_procs;

enum syscall_return_state {
    OK,
    SCHED,
    BLOCK,
    EXIT,
};

enum blocked_state {
    WAIT,
    SLEEP,
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

enum interrupt_request {
    SYS_CREATE,
    SYS_YIELD,
    SYS_EXIT,
    SYS_WAIT_PID,
    SYS_GET_PID,
    SYS_KILL,

    SYS_SLEEP,

    SYS_TIME_SLICE,
};

enum interrupt_source {
    INT_SYSCALL,
    INT_TIMER,
};

typedef struct {
    spinlock_t lock;
    int length;
    // uint64_t avg_throughput;
    // uint64_t avg_l1_refill;
    // uint64_t avg_l2_refill;
    struct list_head tasks[PRIORITY_HIGH + 1];
} ready_queue_t;

typedef struct {
    spinlock_t lock;
    struct list_head tasks;
} wait_queue_t;

// For implementation details see the __load_context routine.
typedef struct {
    uint64_t spsr;    
    uint64_t elr;
    uint64_t reg[32];
} aarch64_frame_t;

typedef struct {
    pid_t pid;
    uint64_t ret;
    uint64_t args;

    // Scheduling
    // TODO: Idle Balancing, busiest -> idleest
    // TODO: load balance without locking? per core input channel/buffer, lazy synchronization? lockless queue or spinlock queue or try-acquire/continue?
    //
    uint8_t affinity; // disallow for some procs...
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

    wait_queue_t *curr_wait_queue;
    wait_queue_t waiting;

    // Scheduling Dependencies, processes blocked on an action by this process
    // struct {
        struct list_head sending;
        struct list_head recving;
        // TODO: Can we add ready list here too, if we want to implement some sort of tree of sucessors
    // };
} process_t, pcb_t;

extern spinlock_t newlib_lock;
extern spinlock_t scheduler_lock;

#ifdef SCHED_AFFINITY
// extern struct list_head ready_queue[NUM_CORES][PRIORITY_HIGH + 1];
extern ready_queue_t ready_queue[NUM_CORES];
#else
extern struct list_head ready_queue[];    
#endif

// Initialization
extern void kernel_init();
extern void kernel_start();
extern void kernel_release_handler();

extern void proc_init();
extern void disp_init();
// extern void intr_init();

// Context Switch
extern void switch_from(process_t *process);
extern void switch_to  (process_t *process);
extern void *align(void *ptr);
// extern void __load_context(void);

// Dispatch and Scheduling
extern process_t *next();
extern void ready(process_t *process);
extern void sleep_on(wait_queue_t *queue, process_t *task);
extern void alert_on(wait_queue_t *queue, bool (*condition)(process_t *curr));
extern void common_interrupt( int interrupt_type );

// Helper Functions
extern enum syscall_return_state proc_create(process_t *proc, void (*func)(), uint64_t stack_size, enum process_priority);
extern enum syscall_return_state proc_tick  (process_t *proc);
extern enum syscall_return_state proc_exit  (process_t *proc);
extern enum syscall_return_state proc_wait  (process_t *proc, pid_t pid);
extern enum syscall_return_state proc_sleep (process_t *proc, unsigned int ms);

// Syscalls
extern pid_t syscreate( void(*func)(void), uint64_t stack_size);
extern pid_t sysgetpid( void );
extern void sysyield( void );
extern void sysexit( void );
extern uint64_t syswaitpid( pid_t pid );
// extern void syskill( pid_t pid, int sig );
extern uint64_t syssleep(unsigned int ms);
extern void sys_printk(char *str);
extern void *sys_malloc(size_t len);

int msb(uint64_t x);
int lsb(uint64_t x);

#endif