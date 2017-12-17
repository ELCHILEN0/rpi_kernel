#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "aux.h"

// Memory-Mapped I/O output
inline void mmio_write(uint32_t reg, uint32_t data)
{
	*(volatile uint32_t*)reg = data;
}

// Memory-Mapped I/O input
inline uint32_t mmio_read(uint32_t reg)
{
	return *(volatile uint32_t*)reg;
}

// Loop <delay> times in a way that the compiler won't optimize away
static void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

uart_t *uart0 = (uart_t *) 0x3F201000;

	// TODO: If issues start to occur look at the GPIO CLK logic in the original reference.
void uart_init() {
    // Disable UART0.
	uart0->cr = 0x00000000;
 
	// Clear pending interrupts.
	uart0->icr = 0x7FF;
 
	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 3000000; Baud = 115200.
 
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	uart0->ibrd = 1;
	uart0->fbrd = 40;
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	// uart0->lcrh = (1 << 4) | (1 << 5) | (1 << 6);
 
	// Mask all interrupts.
	// uart0->imsc =	(1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	//               	(1 << 7) | (1 << 8) | (1 << 9) | (1 << 10);
 
	// Enable UART0, receive & transfer part of UART.
	// uart0->cr = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( uart0->fr & (1 << 5) ) { }
	uart0->dr = c;
}

unsigned char uart_getc()
{    
    // Wait for UART to have received something.
	while ( uart0->fr & (1 << 4) ) { }
	return uart0->dr;
}