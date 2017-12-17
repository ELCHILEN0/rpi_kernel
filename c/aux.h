#include <stddef.h>
#include <stdint.h>

// Memory-Mapped I/O
extern void mmio_write(uint32_t reg, uint32_t data);
extern uint32_t mmio_read(uint32_t reg);


/* http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf */
typedef volatile struct {
    uint32_t dr;
    uint32_t rsr_ecr;
    uint32_t _RESERVED_0[0x10 / sizeof(uint32_t)];
    uint32_t fr;
    uint32_t _RESERVED_1[0x04 / sizeof(uint32_t)];
    uint32_t ilpr;
    uint32_t ibrd;
    uint32_t fbrd;
    uint32_t lcrh;
    uint32_t cr;
    uint32_t ifls;
    uint32_t imsc;
    uint32_t ris;
    uint32_t mis;
    uint32_t icr;
    uint32_t damcr;
    uint32_t _RESERVED_2[0x34 / sizeof(uint32_t)];
    uint32_t itcr;
    uint32_t itip;
    uint32_t itop;
    uint32_t tdr;
} uart_t;

typedef volatile struct {
    // TODO: Implement if necessary
} mini_uart_t;

extern void uart_init();
extern void uart_putc(unsigned char c);
extern unsigned char uart_getc();

extern void uart_putl(unsigned long l);