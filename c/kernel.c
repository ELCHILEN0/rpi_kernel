#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    // Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

    // PIN 18 for output 001.
    gpio->sel[1] |= (1 << 24);
    gpio->clr[0] = (1 << 18);
    
    while(1);

    while(1) {
        volatile int i;

        // pin low, turn OK on
        gpio->clr[0] = (1 << 18);
        // gpio->clr[0] = (1 << 17);

        for (i = 0; i < 500000; i++);

        // pin high, turn OK off
        gpio->set[0] = (1 << 18);
        // gpio->set[0] = (1 << 17);

        for (i = 0; i < 500000; i++);        
    }
}