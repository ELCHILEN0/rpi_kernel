#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "gpio.h"

extern char *__text_start;
extern char *__text_end;
extern char *__data_start;
extern char *__data_end;
extern char *__bss_start;
extern char *__bss_end;
extern char *__heap_start;
extern char *__heap_end;
extern char *__stack_start;
extern char *__stack_end;
extern char *__irq_stack_start;
extern char *__irq_stack_end;

extern void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags );

void bss_init() {
    // char *bss = (char*) &__bss_start;

    // while ( (unsigned long) bss < (unsigned long) &__bss_end )
    //     *bss++ = 0;
}

void blink(unsigned int times, unsigned int freq) {
    for (int j = 0; j < times; j++) {
        volatile int i;
        for (i = 0; i < 0x100000 * freq; i++) {
            asm("nop");
        }
        
        SetActLEDState(1);
        for (i = 0; i < 0x100000 * freq; i++) {
            asm("nop");
        }
        SetActLEDState(0);
    }
}

void cstartup( uint32_t r0, uint32_t r1, uint32_t atags ) {
    blink(2, 1);

    if ((int) &gpio->sel[0] == 0x3F200000)
        SetActLEDState(1);
    else
        SetActLEDState(0);

    for (int i = 0; i < 32; i++) {
        gpio_fsel(i, fsel_output);
        gpio_write(i, i % 2 == 0);
    }

    SetActLEDState(0);

    for (int i = 0; i < 32; i++) {
        gpio_fsel(i, fsel_output);
        if (gpio_read(i) == i % 2 == 0) {
            SetActLEDState(1);
            break;
        } else {
            SetActLEDState(0);
        }
    }

    // printf(".text:  [0x%.5X, 0x%.5X]\n", &__text_start, &__text_end);
    // printf(".data:  [0x%.5X, 0x%.5X]\n", &__data_start, &__data_end);
    // printf(".bss:   [0x%.5X, 0x%.5X]\n", &__bss_start, &__bss_end);
    // printf(".heap:  [0x%.5X, 0x%.5X]\n", &__heap_start, &__heap_end);
    // printf(".stack: [0x%.5X, 0x%.5X]\n", &__stack_start, &__stack_end);
    // printf(".irq_s: [0x%.5X, 0x%.5X]\n", &__irq_stack_start, &__irq_stack_end);

    // bss_init();
    // uart_init();

    // kernel_main( r0, r1, atags );
    
    while (1) { }
}

