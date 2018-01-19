#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "interrupts.h"

#include "gpio.h"
#include "timer.h"
#include "peripheral.h"

// TODO: Unhandled exceptions to begin...
interrupt_vector_t vector_table_svc[256];
interrupt_vector_t vector_table_irq[26];

void trap() {
    printf("[interrupt] Unhandled Interrupt\r\n");
}

void init_vector_tables() {
    for (int i = 0; i < 256; i++) {
        if (i < 26) register_interrupt_handler(vector_table_irq, i, &trap);
        register_interrupt_handler(vector_table_svc, i, &trap);
    }
}

void register_interrupt_handler(interrupt_vector_t vector_table[], unsigned int i, void (*handler)()) {
    vector_table[i].handler = handler;
}

void interrupt_svc(int code) {
    printf("[interrupt] Supvervisor Call (0x%X)\r\n", code);

    interrupt_vector_t vector = vector_table_svc[code];
    vector.handler();
}

void interrupt_irq() {
    // printf("[interrupt] IRQ\r\n");

    // __disable_interrupts();


    uint32_t irq_src = mmio_read(0x40000060);

    for (int code = 0; code < 26; code++) {
        if ((irq_src & (1 << code)) != 0) {
            interrupt_vector_t vector = vector_table_irq[code];
            vector.handler();
        }
    }

    if (irq_src == (1 << 11)) {
        static bool next_blinker_state = true;

        local_timer_reset();

        gpio_write(13, next_blinker_state);
        next_blinker_state = !next_blinker_state;
    } else {
        printf("[interupt] Unhandled IRQ\r\n");
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