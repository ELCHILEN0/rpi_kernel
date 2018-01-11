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
    // OK status
    SetActLEDState(1);  

    gpio_fsel(5, sel_input);
    gpio_fsel(6, sel_output);
    gpio_fsel(13, sel_output);

    while(true) {
        gpio_write(6, gpio_read(5));
        gpio_write(13, gpio_read(5));
    }

    // Error status
    SetActLEDState(0);

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

