
.global interrupt_handler_swi
interrupt_handler_swi:
  push {r0-r12, lr} // TODO: This should be implicit by ARM interrupt... (possible stack problem)
  bl interrupt_swi
  pop {r0-r12, lr}
  eret

.global interrupt_handler_irq
interrupt_handler_irq:
  push {r0-r12, lr}
  bl interrupt_irq
  pop {r0-r12, lr}
  subs pc, lr, #4
