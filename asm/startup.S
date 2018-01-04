.globl _start
_start:
    b _reset
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang

_reset:
    mov sp,#0x08000000

    /* Finally start */
    bl cstartup

hang:
    bl hang

.globl dummy
dummy:
    ret