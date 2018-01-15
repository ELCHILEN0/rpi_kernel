#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "gpio.h"

// __attribute__ ((interrupt ("UDEF"))) void interrupt_udef() {
//     printf("UDEF\n");
// }

// typedef struct {
//     void (*handler)();
// } interrupt_descriptor_table_t, idt_t;

// extern idt_t idt[256];

void __attribute__ ((interrupt ("SWI"))) interrupt_swi() {
    volatile unsigned int i_code; // TODO: FIX

    asm("ldr r0, [lr, #-4]");
    asm("bic r0, #0xFF000000");
    asm("mov %0, r0" : "=r"(i_code) : );

    // printf("SWI %x %d\n", i_code, i_code);
    // idt[i_code].handler();
    if (i_code != 0x80) {
        gpio_write(13, true);
    }
}

// __attribute__ ((interrupt ("PABT"))) void interrupt_pabt() {
//     printf("PABT\n");
// }

// __attribute__ ((interrupt ("FIQ"))) void interrupt_fiq() {
//     printf("FIQ\n");
// }

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
    // printf("IRQ\n");
    while (1) {}
}

// __attribute__ ((interrupt ("DABT"))) void interrupt_dabt() {
//     printf("DABT\n");
// }
