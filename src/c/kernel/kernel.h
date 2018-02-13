#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <stdio.h>

#include "list.h"

#include "../multicore.h"

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

enum ctsw_code {
    SYS_CREATE,
    SYS_YIELD,
    SYS_EXIT,
    SYS_WAIT_PID,
    SYS_GET_PID,
    SYS_KILL,
    INT_TIMER,
};

typedef struct {
    uint32_t reg[13];
    // uint32_t sp; (ignored since implicitly set)
    uint32_t lr;
    // uint32_t pc; (on return pc = lr ...)
    // uint32_t cpsr; (ARM handles this, initial setup should be done)
    // union {
    //     uint32_t spsr;
        uint32_t stack_slots[0]; // TODO: Cleanup function
    // }; // TODO: Offset of (Generic stack)
} arm_frame32_t;

typedef struct {
    uint64_t spsr; // SPSR is actually 32 bits but this is for alignment...    
    uint64_t elr;
    uint64_t reg[31];
} aarch64_frame_t;

typedef struct {
    pid_t pid;
    uint32_t ret;
    uint32_t args;

    enum process_priority initial_priority;
    enum process_priority current_priority;
    
    /* Stack */
    uint32_t    stack_size;
    uint32_t    *stack_base;
    aarch64_frame_t   *frame; // stack pointer...

    struct list_head process_list;
    struct list_head process_hash_list;
    struct list_head sched_list;
} process_t, pcb_t;

extern spinlock_t print_lock;

extern void kernel_init();
extern void process_init();
extern void dispatcher_init();
extern void dispatch();

extern void ready(process_t *process);
extern enum ctsw_code context_switch(pcb_t *process);

extern int create(void (*func)(), int stack_size, enum process_priority);

extern pid_t syscreate( void(*func)(void), uint32_t stack_size);
extern pid_t sysgetpid( void );
#endif