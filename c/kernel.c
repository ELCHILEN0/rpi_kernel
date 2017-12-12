#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    // Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

    printf("Welcome to the Kernel!\n");

    char *buf = malloc(16 * sizeof(char));
    char *buf2 = malloc(16 * sizeof(char));

    printf("buf: 0x%X, buf2: 0x%X\n", buf, buf2);

    while (1) {
        read(0, buf, 1);
        write(0, buf, 1);
    }
}