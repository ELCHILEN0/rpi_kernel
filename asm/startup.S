.globl _start
_start:
    LDR sp, =__stack_end
    BL cstartup
    B .
    