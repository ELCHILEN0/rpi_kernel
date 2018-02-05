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

static uint32_t kernel_stack, process_stack, prev_stack;
static uint32_t ret_code, args, interrupt_type;

enum ctsw_code context_switch(pcb_t *process) {
    (void)kernel_stack; 

    // TODO Syscall ret code...
    // ret_code = process->ret;
    // process->frame->reg[0] = ret_code;

    process_stack = (uint32_t) process->frame;

    __spin_lock(&print_lock);
    printf("context switch gdb hook...\r\n");
    __spin_unlock(&print_lock);  
    bool debug = true;
    while(debug);

    // Push kernel r0-r12 (general registers)
    asm volatile("PUSH {r0-r12}");
    // asm volatile("STMIA sp!, {sp}");
    // asm volatile("STMIA sp!, {r0-r12}");
    // Swap kernel -> process stacks
    asm volatile(".global _kernel_to_process    \n\
    _kernel_to_process:                         \n\
        MOV %0, sp          \n\
        MOV sp, %1          \n\
    "   : "=r" (kernel_stack)
        : "r" (process_stack));
    // Restore process r0-pc (all registers), restoring pc jumps to the function
    asm volatile("LDMIA sp!, {r0-pc}^");

    // Interrupt routine pushes registers (TODO verify and adjust)
    asm volatile(".global _int_svc  \n\
    _int_svc:                       \n\
        mov %0, #1          \n\
        mov %1, r1          \n\
        mov %2, r2          \n\
    "   : "=r" (interrupt_type), "=r" (ret_code), "=r" (args));
    // Swap process -> kernel stacks
    asm volatile(".global _process_to_kernel    \n\
    _process_to_kernel:                         \n\
        MOV %0, sp           \n\
        MOV sp, %1           \n\
    "   : "=r" (process_stack)
        : "r" (kernel_stack));
    // Restore kernel r0-r12 (general registers)
    // asm volatile("LDMDB sp!, {r0-r12}^");
    // asm volatile("LDMDB sp,  {sp}^");
    asm volatile("POP {r0-r12}");

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
  
    process->frame = (arm_frame32_t *) process_stack;

    // switch (interrupt_type) {
    //     case 0:
    //         process->ret = ret_code;
    //         //process->ret = process->stack_frame->eax;
    //         ret_code = INT_TIMER;
    //     break;

    //     case 1:
    //         process->args = args;
    //     break;
    // }

    // process->stack_frame = (arm_frame32_t *) process_stack;

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
