#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <stdio.h>

#include "list.h"

#include "../multicore.h"
#include "../timer.h"
#include "../mailbox.h"
#include "../interrupts.h"

// Hash Function from: http://www.tldp.org/LDP/lki/lki-2.html
#define PIDHASH_SZ (4096 >> 6)
#define pid_hashfn(x)   ((((x) >> 8) ^ (x)) & (PIDHASH_SZ - 1))

#define PROC_STACK (4096 * 4)

enum process_priority {
    PRIORITY_IDLE,
    PRIORITY_LOW,
    PRIORITY_MED,
    PRIORITY_HIGH,
};

enum process_state {
    RUNNABLE,
    BLOCKED,
    STOPPED,
};

enum interrupt_request {
    SYS_CREATE,
    SYS_YIELD,
    SYS_EXIT,
    SYS_WAIT_PID,
    SYS_GET_PID,
    SYS_KILL,
    SYS_TIME_SLICE,
};

enum interrupt_source {
    INT_SYSCALL,
    INT_TIMER,
};

typedef struct {
    uint64_t spsr;    
    uint64_t elr;
    uint64_t reg[31];
} aarch64_frame_t;

typedef struct {
    pid_t pid;
    uint64_t ret;
    uint64_t args;

    // enum process_state state;

    enum process_priority initial_priority;
    enum process_priority current_priority;
    
    /* Stack */
    uint64_t    stack_size;
    uint64_t    *stack_base;
    aarch64_frame_t   *frame; // stack pointer...

    struct list_head process_list;
    struct list_head process_hash_list;
    struct list_head sched_list;
} process_t, pcb_t;

extern spinlock_t print_lock;

extern void kernel_init();
extern void kernel_start();
extern void kernel_release_handler();

extern void process_init();
extern void dispatcher_init();

// Context Switch Specific
extern void switch_from(process_t *process);
extern void switch_to  (process_t *process);
extern void __load_context(void);

// Dispatch and Scheduling
extern process_t *next();
extern void ready(process_t *process);
extern void common_interrupt( int interrupt_type );

// Helper Functions
extern int create(void (*func)(), uint64_t stack_size, enum process_priority);

extern pid_t syscreate( void(*func)(void), uint64_t stack_size);
extern pid_t sysgetpid( void );
extern void sysyield( void );
extern void sysexit( void );
extern uint64_t syswaitpid( pid_t pid );
// extern void syskill( pid_t pid, int sig );

#endif