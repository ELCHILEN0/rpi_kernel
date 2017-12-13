#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

__attribute__ ((interrupt ("UDEF"))) void interrupt_udef() {
    printf("UDEF\n");
}

typedef struct {
    void (*handler)();
} interrupt_descriptor_table_t, idt_t;

extern interrupt_descriptor_table_t idt[256];

void __attribute__ ((interrupt ("SWI"))) interrupt_swi() {
    volatile unsigned int i_code;

    asm("ldr r0, [lr, #-4]");
    asm("bic r0, #0xFF000000");
    asm("mov %0, r0" : "=r"(i_code) : );

    printf("SWI %x %d\n", i_code, i_code);
    idt[i_code].handler();
}

__attribute__ ((interrupt ("PABT"))) void interrupt_pabt() {

}

__attribute__ ((interrupt ("FIQ"))) void interrupt_fiq() {

}

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {

}

__attribute__ ((interrupt ("DABT"))) void interrupt_dabt() {

}
