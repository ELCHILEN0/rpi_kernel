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
    while(true);        
}

void interrupt_pabt() {
    uint32_t ifsr;
    asm("MRC p15, 0, %0, c5, c0, 1" :: "r" (ifsr));

    char *fault_status[16] = {
        "No function, reset value",
        "Alignment fault",
        "Debug event fault",
        "Access Flag fault on Section",
        "No function[b]",
        "Translation fault on Section",
        "Access Flag fault on Page",
        "Translation fault on Page",
        "Precise External Abort",
        "Domain fault on Section",
        "No function",
        "Domain fault on Page",
        "External abort on translation, first level",
        "Permission fault on Section",
        "External abort on translation, second level",
        "Permission fault on Page",
    };

    uint32_t code = (ifsr & (0b1111));

    printf("[interrupt] Prefetch Abort - (0x%X = %d) \r\n", ifsr, code, fault_status[code]);
    while(true);
}

void interrupt_dabt() {
    uint32_t dfsr;
    asm("MRC p15, 0, %0, c5, c0, 0" :: "r" (dfsr));

    char *fault_status[25] = {
        "No function, reset value"
        "Alignment fault"
        "Debug event fault"
        "Access Flag fault on Section"
        "Cache maintenance operation fault"
        "Translation fault on Section"
        "Access Flag fault on Page"
        "Translation fault on Page"
        "Precise External Abort"
        "Domain fault on Section"
        "No function"
        "Domain fault on Page"
        "External abort on translation, first level"
        "Permission fault on Section"
        "External abort on translation, second level"
        "Permission fault on Page"
        "No function"
        "No function"
        "Imprecise External Abort"
        "No function"
        "No function"
    };

    uint32_t code = (dfsr & (1 << 10)) >> 6;
    code |= (dfsr & (0b1111));

    printf("[interrupt] Data Abort - (0x%X = %d) %s\r\n", dfsr, code, fault_status[code]);
    while(true);    
}