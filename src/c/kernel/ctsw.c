/* ctsw.c : context switcher
 */

// #include <xeroskernel.h>
// #include <i386.h>
#include "kernel.h"


/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
*/
void _HardwareEntryPoint( void );
void _KernelEntryPoint( void );

extern void enter_el0();

enum ctsw_code context_switch(pcb_t *process) {
    // TODO Syscall ret code...
    // ret_code = process->ret;
    // process->frame->reg[0] = ret_code;

    uint64_t interrupt_type, ret_code, args;

    // Save kernel state...
    asm volatile(".global _kernel_save  \n\
    _kernel_save:                       \n\
        STP X0, X1, [SP, #-16]!     \n\
        STP X2, X3, [SP, #-16]!     \n\
        STP X4, X5, [SP, #-16]!     \n\
        STP X6, X7, [SP, #-16]!     \n\
        STP X8, X9, [SP, #-16]!     \n\
        STP X10, X11, [SP, #-16]!   \n\
        STP X12, X13, [SP, #-16]!   \n\
        STP X14, X15, [SP, #-16]!   \n\
        STP X16, X17, [SP, #-16]!   \n\
        STP X18, X19, [SP, #-16]!   \n\
        STP X20, X21, [SP, #-16]!   \n\
        STP X22, X23, [SP, #-16]!   \n\
        STP X24, X25, [SP, #-16]!   \n\
        STP X26, X27, [SP, #-16]!   \n\
        STP X28, X29, [SP, #-16]!   \n\
        STR X30, [SP, #-16]!        \n\
                                    \n\
        MRS	X1, SPSR_EL1            \n\
        MRS	X2, ELR_EL1             \n\
        STP X1, X2, [SP, #-16]!     \n\
    ");

    // Switch to SP0, Load process SP, Return to Handler
    asm volatile(".global _kernel_to_process    \n\
    _kernel_to_process:                         \n\
        MSR SPSel, #0       \n\
        MOV SP, %0          \n\
        LDP X1, X2, [SP], #16       \n\
        MSR ELR_EL1,    X2          \n\
        MSR SPSR_EL1,   X1          \n\
                                    \n\
        LDR X30, [SP], #16          \n\
        LDP X28, X29, [SP], #16     \n\
        LDP X26, X27, [SP], #16     \n\
        LDP X24, X25, [SP], #16     \n\
        LDP X22, X23, [SP], #16     \n\
        LDP X20, X21, [SP], #16     \n\
        LDP X18, X19, [SP], #16     \n\
        LDP X16, X17, [SP], #16     \n\
        LDP X14, X15, [SP], #16     \n\
        LDP X12, X13, [SP], #16     \n\
        LDP X10, X11, [SP], #16     \n\
        LDP X8, X9, [SP], #16       \n\
        LDP X6, X7, [SP], #16       \n\
        LDP X4, X5, [SP], #16       \n\
        LDP X2, X3, [SP], #16       \n\
        LDP X0, X1, [SP], #16       \n\
    "   : "=r" (process->frame));

    asm volatile("ERET");

    // Interrupt leaves us here, read args from stack
    asm volatile(".global _int_syscall          \n\
    _int_syscall:                               \n\
        MOV %0, #1          \n\
        MOV %1, x1          \n\
        MOV %2, x2          \n\
    "   : "=r" (interrupt_type), "=r" (ret_code), "=r" (args));

    // Save process SP, Switch to SP1
    asm volatile(".global _process_to_kernel    \n\
    _process_to_kernel:                         \n\
        MOV %0, SP          \n\
        MSR SPSel, #1        \n\
    " :: "r" (process->frame));

    // Load kernel state...
    asm volatile(".global _kernel_load          \n\
    _kernel_load:                               \n\
        LDP X1, X2, [SP], #16       \n\
        MSR ELR_EL1,    X2          \n\
        MSR SPSR_EL1,   X1          \n\
                                    \n\
        LDR X30, [SP], #16          \n\
        LDP X28, X29, [SP], #16     \n\
        LDP X26, X27, [SP], #16     \n\
        LDP X24, X25, [SP], #16     \n\
        LDP X22, X23, [SP], #16     \n\
        LDP X20, X21, [SP], #16     \n\
        LDP X18, X19, [SP], #16     \n\
        LDP X16, X17, [SP], #16     \n\
        LDP X14, X15, [SP], #16     \n\
        LDP X12, X13, [SP], #16     \n\
        LDP X10, X11, [SP], #16     \n\
        LDP X8, X9, [SP], #16       \n\
        LDP X6, X7, [SP], #16       \n\
        LDP X4, X5, [SP], #16       \n\
        LDP X2, X3, [SP], #16       \n\
        LDP X0, X1, [SP], #16       \n\
    ");

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

/*
 * Initialize the interupt entry point when context switching.
 */
void context_init() {
    // set_evec((unsigned int) KERNEL_INT, (unsigned long) _KernelEntryPoint);
    // set_evec((unsigned int) TIMER_INT,  (unsigned long) _HardwareEntryPoint);
    // set_evec((unsigned int) KEYBOARD_INT, (unsigned long) _KeyboardEntryPoint);
    // initPIT( 10 * TIMER_SLICE );
}
