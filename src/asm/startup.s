.section .init

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

.global _vectors
_vectors:
    b _reset
    b interrupt_handler_undef
    b interrupt_handler_swi
    b interrupt_handler_pftch
    b interrupt_handler_abt
    nop
    b interrupt_handler_irq
    b interrupt_handler_fiq

_reset:
  bl __disable_interrupts

  /**
   * Instead of copying the vector table to 0x0, update the vector base register
   */
  ldr     r4, =_vectors
  mov sp, #(60 * 1024 * 1024)
  mcr     p15, #0, r4, c12, c0, #0

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

  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  mov sp, #(62 * 1024 * 1024)

  mov r0, #(CPSR_MODE_ABORT | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  mov sp, #(63 * 1024 * 1024)

  mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  mov sp, #(64 * 1024 * 1024)

  b main

.section .text
main:
  bl cstartup

hang:
  b hang

.global __enable_interrupts
__enable_interrupts:
  cpsie i
  bx lr

.global __disable_interrupts
__disable_interrupts:
  cpsid aif
  bx lr

