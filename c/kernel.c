#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    // Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

    printf("Welcome to the Kernel!\n");

    if (false) {
        // libc malloc test
        char *buf = malloc(16 * sizeof(char));
        char *buf2 = malloc(16 * sizeof(char));

        printf("buf: 0x%X, buf2: 0x%X\n", buf, buf2);
    }

    printf("Interrupts...");
    enable_intrs();
    printf(" enabled.\n");

    // enable_fiq();
    asm("swi 0x80");
    asm("swi 0x90");
    asm("swi 0x10");
    asm("swi 0x20");

    char buf[16];
    while (1) {
        read(0, buf, 1);
        write(0, buf, 1);
    }
}