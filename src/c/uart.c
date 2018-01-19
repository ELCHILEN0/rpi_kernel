#include "uart.h"
#include "peripheral.h"

#include "gpio.h"

aux_t *aux = (aux_t *) AUX_BASE;
// uart_t *uart0 = (uart_t *) UART0_BASE;

void uart_init( unsigned int baudrate ) {
	aux->enables &= ~(AUX_ENABLES_SP1 | AUX_ENABLES_SP2);
	
	gpio_fsel(14, SEL_ALT5);
	gpio_fsel(15, SEL_ALT5);

	aux->uart1.iir |= MU_IIR_SEND_FIFO_CLR | MU_IIR_RECV_FIFO_CLR;
	aux->uart1.lcr |= MU_LCR_8_BIT;
	aux->uart1.ier |= MU_IER_SEND | MU_IER_RECV;
	aux->uart1.baud = ((SYS_FREQ / (8 * baudrate)) - 1) & MU_BAUD_MASK;

	aux->enables |= AUX_ENABLES_MU;
}

// TODO: Make dependent
void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ((aux->uart1.lsr & MU_LSR_SEND_EMPTY) == 0) { }
	aux->uart1.io = c & MU_IO_MASK;

	// while ( uart0->fr & (1 << 5) ) { }
	// uart0->dr = c;
}

unsigned char uart_getc()
{    
    // Wait for UART to have received something.


	// while ( uart0->fr & (1 << 4) ) { }
	// return uart0->dr;
}