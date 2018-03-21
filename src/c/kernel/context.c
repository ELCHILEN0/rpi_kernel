#include "include/kernel/context.h"
#include "include/kernel/sched.h"

/*
 * The switch_from funciton shall save a process stack.
 */
void switch_from(task_t *task) {
    uint64_t stack_pointer;
    asm volatile(".global _switch_from    \n\
    _switch_from:                         \n\
        MSR SPSel, #0       \n\
        MOV %0, SP          \n\
        MSR SPSel, #1       \n\
    " : "=r" (stack_pointer));

    task->frame = (aarch64_frame_t *) stack_pointer;
}

/*
 * The switch_to function shall load a process stack to be restored on an ERET.
 */
void switch_to(task_t *task) {
    // Place previous interrupt return code in x0
    task->frame->reg[0] = task->ret;

    asm volatile(".global _switch_to    \n\
    _switch_to:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
        MSR SPSel, #1       \n\
    " :: "r" (task->frame));
}