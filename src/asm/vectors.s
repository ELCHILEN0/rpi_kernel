
.global interrupt_handler_swi
.global interrupt_handler_undef
.global interrupt_handler_pftch
.global interrupt_handler_abt
.global interrupt_handler_fiq
.global interrupt_handler_irq

interrupt_handler_swi:
  push {r0-r12, lr} // TODO: This should be implicit by ARM interrupt... (possible stack problem)
  bl interrupt_swi
  pop {r0-r12, lr}
  //cpsie aif eret handles this....
  eret

interrupt_handler_fiq:
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

/* ARM 11.3

MOVS PC, R14 
MOVS PC, R14 
SUBS PC, R14, #4 
SUBS PC, R14, #8 
SUBS PC, R14, #4 
SUBS PC, R14, #4
*/