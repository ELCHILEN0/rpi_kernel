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
#include "multicore.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);

extern void _init_core_1(void);
extern void _init_core_2(void);
extern void _init_core_3(void);

void slave_core() {
    switch (get_core_id()) {
        case 1:
            gpio_write(13, true);
            break;

        case 2:
            gpio_write(19, true);
            break;

        case 3:
            gpio_write(21, true);
            break;

        default:
            printf("????????????");
            break;
    }
    
    while (true);
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
	(void) r0;
	(void) r1;
	(void) atags;

    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    /* Initialization */
    gpio_fsel(5, SEL_INPUT);
    gpio_fsel(6, SEL_OUTPUT);
    gpio_fsel(13, SEL_OUTPUT);
    gpio_fsel(19, SEL_OUTPUT);
    gpio_fsel(21, SEL_OUTPUT);

    uart_init(9600);

    printf("[kernel] Running on core %d\r\n", get_core_id());
    gpio_write(6, true);
    core_enable(1, (uint32_t) _init_core_1);
    core_enable(2, (uint32_t) _init_core_2);
    core_enable(3, (uint32_t) _init_core_3);

    printf("[kernel] ........\r\n");
    while (true);

    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}