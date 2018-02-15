#include "kernel.h"

void identify_and_clear_source() {
    // TODO: Adapt for other interrupt types...
    asm volatile(".global _identify_and_clear_source    \n\
    _identify_and_clear_source:                         \n\
        MSR SPSel, #1               \n\
        STP X0, X1, [SP, #-16]!     \n\
        MOV X0, #1                  \n\
        STR X0, [SP, #-8]!          \n\
    " ::: "x0", "x1");
}

enum interrupt_source context_switch(pcb_t *process) {
    uint64_t interrupt_type, ret_code, args, stack_pointer;

    asm volatile(".global _kernel_save  \n\
    _kernel_save:                       \n\
        MSR SPSel, #1               \n\
        STR X30, [SP, #-8]!         \n\
        STP X28, X29, [SP, #-16]!   \n\
        STP X26, X27, [SP, #-16]!   \n\
        STP X24, X25, [SP, #-16]!   \n\
        STP X22, X23, [SP, #-16]!   \n\
        STP X20, X21, [SP, #-16]!   \n\
        STP X18, X19, [SP, #-16]!   \n\
        STP X16, X17, [SP, #-16]!   \n\
        STP X14, X15, [SP, #-16]!   \n\
        STP X12, X13, [SP, #-16]!   \n\
        STP X10, X11, [SP, #-16]!   \n\
        STP X8, X9, [SP, #-16]!     \n\
        STP X6, X7, [SP, #-16]!     \n\
        STP X4, X5, [SP, #-16]!     \n\
        STP X2, X3, [SP, #-16]!     \n\
        STP X0, X1, [SP, #-16]!     \n\
                                    \n\
        MRS	X9, SPSR_EL1            \n\
        MRS	X10, ELR_EL1            \n\
        STP X9, X10, [SP, #-16]!    \n\
    " ::: "x9", "x10");

    // Place previous interrupt return code in x0
    process->frame->reg[0] = process->ret;

    // Switch to SP0, Load Process SP
    asm volatile(".global _kernel_to_process    \n\
    _kernel_to_process:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
    " :: "r" (process->frame));
    
    asm volatile(".global _process_load \n\
    _process_load:                      \n\
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

    // NOTE: ESR = 0x560000XX == aarch64 SVC exception (XX == code)
    // TODO: move this to interrupt identification function...
    // Store syscall params, and interrupt type id to the process stack
    asm volatile(".global _int_syscall          \n\
    _int_syscall:                               \n\
        MSR SPSel, #0                   \n\
        STP X0, X1, [SP, #-16]!         \n\
        MOV X0, #1                      \n\
        STR X0,     [SP, #-8]!          \n\
    "); 

    // Load kernel state...
    asm volatile(".global _kernel_load          \n\
    _kernel_load:                               \n\
        MSR SPSel, #1               \n\
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

    // Handle the exception, saving the process SP and loading the syscall args
    asm volatile(".global _process_save         \n\
    _process_save:                              \n\
        MSR SPSel, #0       \n\
        LDR %0,     [SP], #8    \n\
        LDP %1, %2, [SP], #16   \n\
        MOV %3, SP          \n\
        MSR SPSel, #1       \n\
    " : "=r" (interrupt_type), "=r" (ret_code), "=r" (args), "=r" (stack_pointer));

    process->frame = (aarch64_frame_t *) stack_pointer;

    switch (interrupt_type) {
        case 1:
            process->ret = ret_code;
            process->args = args;
        break;

        default:
            __spin_lock(&print_lock);
            printf("[kernel] Unhandled context switch...\r\n");
            __spin_unlock(&print_lock);
        break; 
    }
    
    return ret_code;
}

void context_init() {
    // set_evec((unsigned int) KERNEL_INT, (unsigned long) _KernelEntryPoint);
    // set_evec((unsigned int) TIMER_INT,  (unsigned long) _HardwareEntryPoint);
    // set_evec((unsigned int) KEYBOARD_INT, (unsigned long) _KeyboardEntryPoint);
    // initPIT( 10 * TIMER_SLICE );
}
