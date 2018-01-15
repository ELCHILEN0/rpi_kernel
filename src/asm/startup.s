.section .init
.global _vectors

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

_vectors:
    b _reset
    nop
    b interrupt_handler_swi
    nop
    nop
    nop
    b interrupt_handler_irq
    nop

_reset:
  ldr     r4, =_vectors
  mcr     p15, #0, r4, c12, c0, #0

  // We start on hypervisor mode. Switch back to SVR
  mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr spsr_c,   r0
  add r0, pc, #4
  msr ELR_hyp,  r0
  eret

/*
  // Copy interrupt vectors to 0x0
  mov     r0, #_vectors
  mov     r1, #0x0000
  ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
  stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
  ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
  stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
*/

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

.global enable_intrs
enable_intrs:
  cpsie aif
  bx lr

.global enable_intrs
disable_intrs:
  cpsid aif
  bx lr

