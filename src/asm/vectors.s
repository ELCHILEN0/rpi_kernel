.global interrupt_handler_swi
.global interrupt_handler_undef
.global interrupt_handler_pftch
.global interrupt_handler_abt
.global interrupt_handler_fiq
.global interrupt_handler_irq

/**
 * Exception handling semantics come from ch. 11-12 in ARM Cortex-A Manual.  Strictly
 * speaking, interrupt sources should be cleared before the handler is invoked,
 * this will be important in cases with reentrant interrupts.  For nested interrupts
 * see ch. 12.1.3.
 */

interrupt_handler_swi:
  push {r0-r12, lr}
  ldr r0, [lr, #-4]
  bic r0, #0xFF000000
  bl interrupt_swi
  pop {r0-r12, lr}
  MOVS PC, lr

interrupt_handler_fiq: // TODO: Different handler...
interrupt_handler_irq:
  push {r0-r12, lr}
  bl interrupt_irq
  pop {r0-r12, lr}
  SUBS PC, lr, #4

interrupt_handler_abt:
  push {r0-r12, lr}
  bl interrupt_irq_other
  pop {r0-r12, lr}
  SUBS PC, lr, #8

interrupt_handler_pftch:
  push {r0-r12, lr}
  bl interrupt_irq_other
  pop {r0-r12, lr}
  SUBS PC, lr, #4

interrupt_handler_undef:
  push {r0-r12, lr}
  bl interrupt_irq_other
  pop {r0-r12, lr}
  MOVS PC, lr
