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

    for (i = 0; i < iters; i++);
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    // Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

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