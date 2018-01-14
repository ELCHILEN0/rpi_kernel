#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"

#include "mailbox.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

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

    // OK status
    gpio_write(21, true);
    gpio_write(13, true);

    while(true) {
        bool state = gpio_read(5);
        
        gpio_write(6, state);

        while (state == gpio_read(5));
    }

    // Error status
    gpio_write(21, false);
    // act_message[6] = 0;
    // mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}