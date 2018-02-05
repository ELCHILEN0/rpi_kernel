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
// void _KeyboardEntryPoint( void );

// static uint32_t kernel_stack, process_stack, prev_stack;
// static uint32_t ret_code, args, interrupt_type;

enum ctsw_code context_switch(pcb_t *process) {
    // (void)kernel_stack; 

    // TODO Syscall ret code...
    // ret_code = process->ret;
    // process->frame->reg[0] = ret_code;

    // process_stack = (uint32_t) process->frame;

    __spin_lock(&print_lock);
    printf("context switch gdb hook...\r\n");
    __spin_unlock(&print_lock);  
    bool debug = true;
    while(debug);

    // Push kernel r0-r12 (general registers), sp and lr are implicity stored (banked)
    asm volatile("PUSH {r0-r12}");
    // Switch to USER mode and update sp
    asm volatile(".global _kernel_to_process    \n\
    _kernel_to_process:                         \n\
        MOV r0, #(CPSR_MODE_USER | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT ) \n\
        MSR CPSR_c, r0      \n\
        MOV sp, %0          \n\
    "   : "=r" (process->frame)
        :: "r0");

    // Restore process r0-r12, lr (working set)
    asm volatile("POP {r0-r12, lr}");
    asm volatile("MOVS pc, lr");

    uint32_t interrupt_type, ret_code, args;
    // Switch to SYSTEM mode to save USER sp in process->frame
    // Read USER syscall args ... (r1-r2)
    // Switch back to SVC restoring sp implicitl
    asm volatile(".global _process_to_kernel        \n\
    _process_to_kernel:                             \n\
    .global _int_syscall                            \n\
    _int_syscall:                                   \n\
        MOV r0, #(CPSR_MODE_SYSTEM | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)   \n\
        MSR CPSR_c, r0      \n\
        MOV %3, sp          \n\
        MOV %0, #1          \n\
        MOV %1, r1          \n\
        MOV %2, r2          \n\
        MOV r0, #(CPSR_MODE_SVC | CPSR_IRQ_INHIBT | CPSR_FIQ_INHIBIT)   \n\
        MSR CPSR_c, r0      \n\
    "   : "=r" (interrupt_type), "=r" (ret_code), "=r" (args)
        : "r" (process->frame));
    // Restore kernel r0-r12 (general registers), sp and lr are already restored (banked)
    asm volatile("POP {r0-r12, lr}");

    // process->frame = (arm_frame32_t *) process_stack;    

    debug = true;
    while(debug);

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
