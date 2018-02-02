#ifndef UART_H
#define UART_H

#include <stdint.h>

/*
 * Note: Many of these values are derived from the erata.
 * https://elinux.org/BCM2835_datasheet_errata
 */
typedef enum {
    AUX_ENABLES_MU = (1 << 0),
    AUX_ENABLES_SP1 = (1 << 1),
    AUX_ENABLES_SP2 = (1 << 1),

    MU_IER_RECV = (1 << 0),
    MU_IER_SEND = (1 << 1),
    MU_IIR_RECV_FIFO_CLR = (1 << 1),
    MU_IIR_SEND_FIFO_CLR = (1 << 2),
    MU_LCR_8_BIT = 0x3,
    MU_BAUD_MASK = 0xFFFF,

    MU_LSR_SEND_EMPTY = (1 << 5),

    MU_IO_MASK = 0xFF,
} aux_reg_value_t;

typedef volatile struct {
    uint32_t io;
    uint32_t ier;
    uint32_t iir;
    uint32_t lcr;
    uint32_t mcr;
    uint32_t lsr;
    uint32_t msr;
    uint32_t scratch;
    uint32_t cntl;
    uint32_t stat;
    uint32_t baud;
} mini_uart_t;

typedef volatile struct {
    uint32_t cntl[2];
    uint32_t stat;
    uint32_t io;
    uint32_t peek;
} spi_t;

typedef volatile struct {
    uint32_t irq;
    uint32_t enables;
    uint32_t _RESERVED_0[(0x40 - 0x8) / sizeof(uint32_t)];
    mini_uart_t uart1;
    spi_t spi[2];
} aux_t;

extern aux_t *aux;

/* http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf */
// typedef volatile struct {
//     uint32_t dr;
//     uint32_t rsr_ecr;
//     uint32_t _RESERVED_0[0x10 / sizeof(uint32_t)];
//     uint32_t fr;
//     uint32_t _RESERVED_1[0x04 / sizeof(uint32_t)];
//     uint32_t ilpr;
//     uint32_t ibrd;
//     uint32_t fbrd;
//     uint32_t lcrh;
//     uint32_t cr;
//     uint32_t ifls;
//     uint32_t imsc;
//     uint32_t ris;
//     uint32_t mis;
//     uint32_t icr;
//     uint32_t damcr;
//     uint32_t _RESERVED_2[0x34 / sizeof(uint32_t)];
//     uint32_t itcr;
//     uint32_t itip;
//     uint32_t itop;
//     uint32_t tdr;
// } uart_t;

// extern uart_t *uart0;

extern void uart_init( unsigned int baudrate  );
extern void uart_putc(unsigned char c);
extern unsigned char uart_getc();

#endif