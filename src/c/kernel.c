#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"

void busy_wait(int iters) {
    volatile unsigned int i;

    for (i = 0; i < iters; i++) asm("nop");
}

unsigned int msg[] = {32, 0, 0x00038041, 8, 0, 130, 1, 0};
const unsigned int MAILBOX = 0x3f00b880;
void open_led() {
    unsigned int status;
    do {
        status = *(unsigned int *)(MAILBOX + 0x18);
    } while (status & 0x80000000);
    *(unsigned int *)(MAILBOX + 0x20) = (unsigned int)msg + 8;
    while (1)
        ;
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    // Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

    open_led();

    // PIN 18 for output 001.
    for (int i = 0; i < 32; i++) {
        gpio_fsel(i, fsel_output);
        gpio_write(i, true);
    }

    gpio_fsel(4, fsel_output);
    while (1) {
        gpio_write(4, true);
        busy_wait(3000000);

        gpio_write(4, false);
        busy_wait(3000000);
    }
}