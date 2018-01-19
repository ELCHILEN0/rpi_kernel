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

void interrupt_svc(int code) {
    printf("[interrupt] Supvervisor Call (0x%X)\r\n", code);

    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void interrupt_irq() {
    // printf("[interrupt] IRQ\r\n");

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

void interrupt_fiq() {
    printf("[interrupt] FIQ\r\n");
}

void interrupt_udef() {
    printf("[interrupt] Undefined Instruction\r\n");
}

void interrupt_pabt() {
    printf("[interrupt] Prefetch Abort\r\n");
}

void interrupt_dabt() {
    printf("[interrupt] Data Abort\r\n");
}