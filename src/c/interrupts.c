#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "gpio.h"
#include "timer.h"
#include "peripheral.h"

/**
 * Higher level c-function interrupt methods.
 */


// typedef struct {
//     void (*handler)();
// } interrupt_descriptor_table_t, idt_t;

// extern idt_t idt[256];

// void interrupt_udef() {
//     printf("UDEF\n");
// }

void interrupt_swi(int code) {
    printf("[interrupt] SWI(0x%X)\r\n", code);

    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

// void interrupt_pabt() {
//     printf("PABT\n");
// }

// void interrupt_fiq() {
//     printf("FIQ\n");
// }

void interrupt_irq() {
    // __disable_interrupts();

    static bool next_blinker_state = true;

    uint32_t irq_src = mmio_read(0x40000060);

    if (irq_src == (1 << 11)) {
        local_timer_reset();

        gpio_write(13, next_blinker_state);
        next_blinker_state = !next_blinker_state;
    } else {
        gpio_write(13, true);
        gpio_write(21, true);
        return;
    }

    // __enable_interrupts();    
}

void interrupt_irq_other() {
    gpio_write(13, true);
    gpio_write(21, true);
}

// void interrupt_dabt() {
//     printf("DABT\n");
// }
