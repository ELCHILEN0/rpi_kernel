.global interrupt_handler_svc
.global interrupt_handler_udef
.global interrupt_handler_pabt
.global interrupt_handler_dabt
.global interrupt_handler_fiq
.global interrupt_handler_irq

/**
 * Exception handling semantics come from ch. 11-12 in ARM Cortex-A Manual.  Strictly
 * speaking, interrupt sources should be cleared before the handler is invoked,
 * this will be important in cases with reentrant interrupts.  For nested interrupts
 * see ch. 12.1.3.
 */
interrupt_handler_svc:
  @ TODO: lr = lr
  @ STMIA sp!, {sp-pc}
  @ STMIA sp!, {r0-r12, lr}
  @ push {sp-pc}
  @ push {r0-r12}
  push {r0-r12, lr}
  ldr r0, [lr, #-4]
  bic r0, #0xFF000000
  bl interrupt_svc
  pop {r0-r12, lr}
  MOVS PC, lr

interrupt_handler_udef:
  push {r0-r12, lr}
  bl interrupt_udef
  pop {r0-r12, lr}
  MOVS PC, lr

interrupt_handler_irq:
  SUBS lr, lr, #4
  push {r0-r12, lr}
  bl interrupt_irq
  pop {r0-r12, lr}
  SUBS PC, lr, #4

interrupt_handler_fiq:
  push {r0-r12, lr}
  bl interrupt_fiq
  pop {r0-r12, lr}
  SUBS PC, lr, #4

interrupt_handler_pabt:
  push {r0-r12, lr}
  bl interrupt_pabt
  pop {r0-r12, lr}
  SUBS PC, lr, #4

interrupt_handler_dabt:
  push {r0-r12, lr}
  bl interrupt_dabt
  pop {r0-r12, lr}
  SUBS PC, lr, #8
  