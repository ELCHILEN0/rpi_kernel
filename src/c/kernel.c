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
#include "mailbox.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void enable_intrs(void);

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

    local_timer_interrupt_routing(0);
    local_timer_start(0x038FFFF);

    __enable_interrupts();

    uart_init(9600);
    // asm("swi 0x80"); // Does it work once
    // asm("swi 0x80"); // Does it work again...

    // OK status

    while(true) {
        bool state = gpio_read(5);
        
        uart_putc('H');
        uart_putc('e');
        uart_putc('l');
        uart_putc('l');
        uart_putc('o');
        uart_putc('\r');
        uart_putc('\n');
        gpio_write(6, state);
        asm("swi 0x80");

        while (state == gpio_read(5));
    }

    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}