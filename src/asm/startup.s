.section .init
.global _start

_start:
  mov sp, #0x8000
  b main

.section .text
main:
  bl cstartup

hang:
  b hang
