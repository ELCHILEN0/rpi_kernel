#include "uart.h"
#include "peripheral.h"

extern uart_t *uart0 = (uart_t *) UART0_BASE;

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
	uart0->cr = (1 << 0) | (1 << 8) | (1 << 9);
}

// TODO: Make dependent
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