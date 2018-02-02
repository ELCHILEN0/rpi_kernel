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

typedef struct {
    uint32_t reg[13];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
    uint32_t stack_slots[];
} arm_frame32_t;

typedef struct {
    pid_t pid;
    uint32_t ret;
    
    /* Stack */
    uint32_t    stack_size;
    uint32_t    *stack_base;
    arm_frame32_t   *frame;

    struct list_head process_hash_list;
} process_t, pcb_t;

enum ctsw_code {
    SYS_CREATE,
    INT_TIMER,
};

extern spinlock_t print_lock;

extern void kernel_init();
extern void process_init();

extern enum ctsw_code context_switch(pcb_t *process);

extern int create(void (*func)(void), int stack_size);
#endif