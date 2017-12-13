#include <stddef.h>
#include <stdint.h>

// Memory-Mapped I/O
extern void mmio_write(uint32_t reg, uint32_t data);
extern uint32_t mmio_read(uint32_t reg);

extern void uart_init();
extern void uart_putc(unsigned char c);
extern unsigned char uart_getc();

extern void uart_putl(unsigned long l);