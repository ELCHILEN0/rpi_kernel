.section .text

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

.equ    CORE_0_MBOX_3_SET, 0x4000008C
.equ    CORE_1_MBOX_3_SET, 0x4000009C
.equ    CORE_2_MBOX_3_SET, 0x400000AC
.equ    CORE_3_MBOX_3_SET, 0x400000BC

.global _vectors
_vectors:
    b _reset
    b interrupt_handler_udef
    b interrupt_handler_svc
    b interrupt_handler_pabt
    b interrupt_handler_dabt
    nop
    b interrupt_handler_irq
    b interrupt_handler_fiq

_reset:
    /**
     * Place the other cores in a spin loop until they can be woken
         mov r0, #1
    ldr r1, =#CORE_1_MBOX_3_SET
    b _spin_core
    mov r0, #2
    ldr r1, =#CORE_2_MBOX_3_SET
    b _spin_core
    mov r0, #3
    ldr r1, =#CORE_3_MBOX_3_SET
    b _spin_core
     */
    

    /**
    * Hypervisor mode uses different interrupt vector entries; therefore, we
    * switch back to SVR for predictable execution.
    */
    mrs r0, cpsr
    bic r0, r0, #CPSR_MODE_SYSTEM
    orr r0, r0, #CPSR_MODE_SVR
    msr spsr_cxsf,   r0
    add r0, pc, #4
    msr ELR_hyp,  r0
    eret

    /**
    * Setup exception level stacks, careful memory layout should be used, and
    * these addresses should be made available to the memory allocator.
    */
    mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #(62 * 1024 * 1024)

    mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #(64 * 1024 * 1024)

    /**
    * Finally branch to higher level c routines.
    */
    bl cstartup

_hang:
    b _hang


/**
 * _spin_core(r0 = core_to_spin, r1 = addr_to_watch)
 */
_spin_core:
    mcr p15, 0, r2, c0, c0, 5
    bic r2, #0x3    
    cmp r0, r2  // spin_core == current_core
    bne _spin_core_exit

_spin_core_loop:
    ldr r0, [r1]
    cmp r0, #0
    beq _spin_core_loop

_spin_core_exit:
    ldr pc, [r0]
    
/**
 * __enable_interrupts()
 */
.global __enable_interrupts
__enable_interrupts:
  cpsie aif
  bx lr

/**
 * __disable_interrupts()
 */
.global __disable_interrupts
__disable_interrupts:
  cpsid aif
  bx lr
