#include "include/context.h"

/*
 * The switch_from funciton shall save a process stack.
 */
void switch_from(process_t *process) {
    uint64_t stack_pointer;
    asm volatile(".global _switch_from    \n\
    _switch_from:                         \n\
        MSR SPSel, #0       \n\
        MOV %0, SP          \n\
        MSR SPSel, #1       \n\
    " : "=r" (stack_pointer));

    process->frame = (aarch64_frame_t *) stack_pointer;
}

/*
 * The switch_to function shall load a process stack to be restored on an ERET.
 */
void switch_to(process_t *process) {
    // Place previous interrupt return code in x0
    process->frame->reg[0] = process->ret;

    asm volatile(".global _switch_to    \n\
    _switch_to:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
        MSR SPSel, #1       \n\
    " :: "r" (process->frame));
}

void *align(void *ptr, unsigned int alignment) {
    return (void *) ((uint64_t) ptr & -alignment);
}

void CPU_ZERO(cpu_set_t *set) {
    set->count = 0;
}
void CPU_SET(int cpu, cpu_set_t *set) {
    set->count |= (1 << cpu);
}
void CPU_CLR(int cpu, cpu_set_t *set) {
    set->count &= ~(1 << cpu);
}
int  CPU_ISSET(int cpu, cpu_set_t *set) {
    return set->count & (1 << cpu);
}
int  CPU_COUNT(cpu_set_t *set) {
    int count;
    for (int cpu = 0; cpu < NUM_CORES; cpu++) {
        if (CPU_ISSET(cpu, set))
            count++;
    }
    return count;
}
