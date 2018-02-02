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

extern void idleproc();

enum ctsw_code context_switch(pcb_t *process) {
    (void)kernel_stack; 

    ret_code = process->ret;
    process->frame->reg[0] = ret_code;

    //process_stack = (unsigned long) process->stack_frame; 
 
    /* In the assembly code, switching to process
     * 1.  Push kernel eflags and general registers onto the stack
     * 2.  Save kernel stack pointer
     * 3.  Switch to the process stack
     * 4.  Pop process general registers from the stack
     * 5.  Store return value in eax
     * 6.  Do an iret to switch to process (implicit popf, from int) 
     *
     * Switching to kernel
     * 1.  Read syscall params
     * 2.  Push general registers onto the stack
     * 3.  Save the process stack pointer
     * 4.  Switch to the kernel stack
     * 5.  Pop kernel eflags and general registers from the stack
     */

    __spin_lock(&print_lock);
    printf("0x%X = 0x%X\r\n", idleproc, process->frame->lr);
    __spin_unlock(&print_lock);  
    // idleproc();

    /*
        PUSH {r0-r12, lr}
        MSR r0, CPSR
        PUSH {r0}

        POP {r0}
        MRS CPSR, r0
        POP {r0-r12, lr}
    */

    asm volatile ("BX %0" :: "r" (process->frame->lr));
    asm volatile("\
            STMIA sp, {r0-pc}^ \n\
            MOV sp, %0 \n\
            MOV %1, sp \n\
            LDMIA sp, {r0-pc}^ \n\
        .global _InterruptEntryPoint \n\
        _InterruptEntryPoint: \n\
            nop \n\
        " : "=r" (kernel_stack)
          : "r" (process_stack), "r" (process->frame->lr));

    __spin_lock(&print_lock);
    printf("Back in kernel...\r\n");
    __spin_unlock(&print_lock);  
    while (true);
//     __asm __volatile( " \
//         PUSH {r0-lr}
//         pushf \n\
//         pusha \n\
//         movl    %%esp, kernel_stack	\n\
//         movl    process_stack, %%esp	\n\
//         popa \n\
//         iret \n\
//    _KernelEntryPoint:	\n\
// 	    cli	\n\
//         pusha	\n\
// 	    movl	$1, interrupt_type \n\
//         movl	4(%%ebp), %%edx \n\
//         movl	%%edx, ret_code	\n\
//         movl    8(%%ebp), %%edx \n\
//         movl    %%edx, args     \n\
//  	    jmp	_CommonJump	\n\
//    _HardwareEntryPoint:	\n\
// 	    cli	\n\
//         pusha	\n\
//         movl	$0, interrupt_type	\n\
//    _CommonJump: \n\
//         movl    %%esp, process_stack  \n\
//         movl    kernel_stack, %%esp  \n\
//         popa \n\
//         popf \n\
//         POP {r0-lr, pc}
//         "
//         :
//         : 
//         : "%eax"
//     );
  
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
