.globl enable_intrs
.globl disable_intrs

// See ARM section A2.2 (Processor Modes)
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F

// See ARM section A2.5 (Program status registers)
.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

.equ	SCTLR_ENABLE_DATA_CACHE,        0x4
.equ	SCTLR_ENABLE_BRANCH_PREDICTION, 0x800
.equ	SCTLR_ENABLE_INSTRUCTION_CACHE, 0x1000

/*
 * Interrupt Guides: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0210c/CIHFJDDH.html
 * Each type of interrupt generally has different return semantics
 *
 * ARM Cortex-A53
 */

/*
 * Interrupt vector table, on most CPU's this will reside at 0x0.
 * Thus you can copy this there:
    ldr r0, =_reset
    mov r1, #0x0000
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
 * However, for our purposes we are assuming the state of the processor is OK.
 * Assume: returning from interrupt vector will continue execution where it left off, this may not be true....
 */
_vectors:
    b _reset /* Immediately jump to the reset vector */
    b interrupt_udef
    b interrupt_swi
    b interrupt_pabt
    b interrupt_dabt
    b interrupt_udef
    b interrupt_irq
    b interrupt_fiq

/* Enter Execution in Supervisor Mode */
_reset:
    /* Reconfigure the interrupt vector base, once MMU is active may need to move this to ROM.
     * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344k/Bahfeedc.html
     */
    ldr     r4, =_vectors
    mcr     p15, #0, r4, c12, c0, #0  

    /* Disable interrupts TODO: Fix crash when SWI fired before enable
    bl disable_intrs 
    */
    
    /* Setup User Mode Stacks
     * Modify these to point to MMU agnostic region later, see 0xC0000000
     */
    cpsid aif, #CPSR_MODE_SVR
    mov sp, #0x2000

    cpsid aif, #CPSR_MODE_ABORT
    mov sp, #0x3000

    cpsid aif, #CPSR_MODE_IRQ
    mov sp, #0x4000

    cpsid aif, #CPSR_MODE_SYSTEM
    mov sp, #0x5000

    /* Finally start */
    bl cstartup

hang:
    b hang

/*
 * TODO: Improve with inline void with inline asm
 */
enable_intrs:    
    cpsie aif
    bx lr

disable_intrs:
    cpsid aif
    bx lr