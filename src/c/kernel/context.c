#include "kernel.h"

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
void switch_to(process_t *process)  {
    // Place previous interrupt return code in x0
    process->frame->reg[0] = process->ret;

    asm volatile(".global _switch_to    \n\
    _switch_to:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
        MSR SPSel, #1       \n\
    " :: "r" (process->frame));
}

// Bad function, only call once to mimic exception handler.
void __attribute__ ((naked)) contextswitch() {
        // TODO: at this point could jump to the execption handler return for consistent behaivour without code reproduction...
    asm volatile(".global _context_load \n\
    _context_load:                      \n\
        MSR SPSel, #0               \n\
        LDP X9, X10, [SP], #16      \n\
        MSR ELR_EL1, X10            \n\
        MSR SPSR_EL1, X9            \n\
                                    \n\
        LDP X0, X1, [SP], #16       \n\
        LDP X2, X3, [SP], #16       \n\
        LDP X4, X5, [SP], #16       \n\
        LDP X6, X7, [SP], #16       \n\
        LDP X8, X9, [SP], #16       \n\
        LDP X10, X11, [SP], #16     \n\
        LDP X12, X13, [SP], #16     \n\
        LDP X14, X15, [SP], #16     \n\
        LDP X16, X17, [SP], #16     \n\
        LDP X18, X19, [SP], #16     \n\
        LDP X20, X21, [SP], #16     \n\
        LDP X22, X23, [SP], #16     \n\
        LDP X24, X25, [SP], #16     \n\
        LDP X26, X27, [SP], #16     \n\
        LDP X28, X29, [SP], #16     \n\
        LDR X30, [SP], #8           \n\
    ");

    asm volatile("ERET");
    // This function never returns...
}