#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "uart.h"

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
    char *bss = (char*) &__bss_start;

    while ( (unsigned long) bss < (unsigned long) &__bss_end )
        *bss++ = 0;
}

void cstartup( uint32_t r0, uint32_t r1, uint32_t atags ) {
    printf(".text:  [0x%.5X, 0x%.5X]\n", &__text_start, &__text_end);
    printf(".data:  [0x%.5X, 0x%.5X]\n", &__data_start, &__data_end);
    printf(".bss:   [0x%.5X, 0x%.5X]\n", &__bss_start, &__bss_end);
    printf(".heap:  [0x%.5X, 0x%.5X]\n", &__heap_start, &__heap_end);
    printf(".stack: [0x%.5X, 0x%.5X]\n", &__stack_start, &__stack_end);
    printf(".irq_s: [0x%.5X, 0x%.5X]\n", &__irq_stack_start, &__irq_stack_end);

    bss_init();
    uart_init();

    kernel_main( r0, r1, atags );
    
    while (1) { }
}

