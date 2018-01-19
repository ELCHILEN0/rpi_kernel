#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "interrupts.h"

#include "mailbox.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);

void context_switch() {
    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void time_slice() {
    local_timer_reset();

    static bool next_blinker_state = true;
    gpio_write(13, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
	(void) r0;
	(void) r1;
	(void) atags;

    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    gpio_fsel(5, SEL_INPUT);
    gpio_fsel(6, SEL_OUTPUT);
    gpio_fsel(13, SEL_OUTPUT);
    gpio_fsel(21, SEL_OUTPUT);

    uart_init(9600);

    local_timer_interrupt_routing(0);
    local_timer_start(0x038FFFF);

    printf("[kernel] Enabling interrupts...\r\n");
    __enable_interrupts();
    printf("[kernel] Interrupts enabled.\r\n");

    register_interrupt_handler(vector_table_svc, 0x80, &context_switch);
    register_interrupt_handler(vector_table_svc, 0x81, context_switch);
    register_interrupt_handler(vector_table_irq, 11, time_slice);

    printf("[kernel] Jumping to interrupt 0x80.\r\n");
    asm("SVC 0x80");
    printf("[kernel] Returned from interrupt.\r\n");

    bool output_state = true;

    while(true) {
        gpio_write(6, output_state);
        output_state = !output_state;

        asm("SVC 0x81");

        bool input_state = gpio_read(5);
        while (input_state == gpio_read(5)); // Poll till pressed
        while (input_state != gpio_read(5)); // Poll till unpressed
    }

    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}