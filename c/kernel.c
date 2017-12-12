#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
    printf("Welcome to the Kernel!\n");

    char c;
    char *ptr = NULL;
    size_t alloc_size = 1;
    do {
        read(0, &c, 1);
        printf("%d: %c\n", c, c);
        
        ptr = realloc(ptr, alloc_size);
        if(ptr == NULL) {
            puts("Out of memory!\nProgram halting.");
            for(;;);
        } else {
            printf("new alloc of %d bytes at address 0x%X\n", alloc_size, (unsigned int)ptr);
            alloc_size <<= 1;
        }
    } while (1);
}