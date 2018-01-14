.section .init
.global _start

_start:
@; TODO: setup interrupts, setup interrupt stacks, setup core stacks
  mov sp, #0x8000
  b main

.section .text
main:
  bl cstartup

@; cstartup should never terminate but if it does, hang
hang:
  b hang
