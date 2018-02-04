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

    // idleproc();

    /*
        PUSH {r0-r12, lr}
        MSR r0, CPSR
        PUSH {r0}

        POP {r0}
        MRS CPSR, r0
        POP {r0-r12, lr}
    */

// (43) r0 (/32)
// (44) r1 (/32)
// (45) r2 (/32)
// (46) r3 (/32)
// (47) r4 (/32)
// (48) r5 (/32)
// (49) r6 (/32)
// (50) r7 (/32)
// (51) r8 (/32)
// (52) r9 (/32)
// (53) r10 (/32)
// (54) r11 (/32)
// (55) r12 (/32)
// (56) sp (/32)
// (57) lr (/32)
// (58) pc (/32)
// (59) cpsr (/32)
    process_stack = (uint32_t) process->frame;


    __spin_lock(&print_lock);
    printf("waiting for gdb... idleproc at 0x%X\r\n", idleproc);
    __spin_unlock(&print_lock);  
    bool debug = false;
    while(!debug);
    asm("PUSH {r0}");
    asm("PUSH {r1}");
    asm("PUSH {r2}");
    asm("PUSH {r3}");
    asm("PUSH {r4}");
    asm("PUSH {r5}");
    // asm("STMDB sp!, {r0-r12, lr, pc}");
    asm("MOV %0, sp" : "=r" (kernel_stack));
    asm("MOV sp, %0" :: "r" (process_stack));
    asm("POP {r0-r12}");
    asm("POP {sp}");
    asm("POP {lr}");
    asm("POP {pc}");
    // asm("LDMIA sp!, {r0-r12, lr, pc}^");
    asm("BX lr");

    asm volatile("\
        _kernel_to_process: \n\
            LDM sp, {r0-pc} \n\
            BX %2 \n\
        .global _int_svc \n\
        _int_svc: \n\
            nop \n\
        _process_to_kernel: \n\
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
