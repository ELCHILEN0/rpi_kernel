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
#include "../timer.h"
#include "../mailbox.h"
#include "../multicore.h"
#include "../interrupts.h"

#define NUM_CORES 4
#define NUM_TICKS 19200000

// Hash Function from: http://www.tldp.org/LDP/lki/lki-2.html
#define PIDHASH_SZ (4096 >> 6)
#define pid_hashfn(x)   ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

#define PROC_STACK (4096 * 4)

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
    enum process_state state;    
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

    // Timers
    uint64_t tick_count;
    uint64_t tick_delta;

    struct list_head process_list;
    struct list_head process_hash_list;
    struct list_head sched_list;
    struct list_head block_list;

    // TODO: waiters
    // blocked_on is an entry in a process waiters list...
    // can sched_list be merged with block_list and blocked_on...
    // if I am blocked blocked_on will point to the first process I am blocked on (eg p2->blocked_waiters)
    enum blocked_state blocked_cause;
    // struct list_head blocked_on;
    spinlock_t blocked_waiters_lock;
    struct list_head blocked_waiters;
} process_t, pcb_t;

extern spinlock_t newlib_lock;
extern spinlock_t scheduler_lock;

extern struct list_head ready_queue[];
extern struct list_head sleep_queue[];
extern struct list_head block_queue;

// Initialization
extern void kernel_init();
extern void kernel_start();
extern void kernel_release_handler();

extern void proc_init();
extern void disp_init();

// Context Switch
extern void switch_from(process_t *process);
extern void switch_to  (process_t *process);
// extern void __load_context(void);

// Dispatch and Scheduling
extern process_t *next();
extern void ready(process_t *process);
extern void block(process_t *process);
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

int msb(uint64_t x);
int lsb(uint64_t x);

#endif