#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

__attribute__ ((interrupt ("UDEF"))) void interrupt_udef() {
    printf("UDEF\n");
}

void __attribute__ ((interrupt ("SWI"))) interrupt_swi() {
    volatile unsigned int r0;

    asm("LDR r0, [lr, #-4]");
    asm("BIC r0, #0xFF000000");
    asm("MOV %0, r0":"=r"(r0):);

    printf("SWI %x\n", r0);
}

__attribute__ ((interrupt ("PABT"))) void interrupt_pabt() {

}

__attribute__ ((interrupt ("FIQ"))) void interrupt_fiq() {

}

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {

}

__attribute__ ((interrupt ("DABT"))) void interrupt_dabt() {

}
