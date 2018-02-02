#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>

#define PERIPHERAL_BASE 0x3F000000

#define MAILBOX_BASE    (PERIPHERAL_BASE | 0xB880)

#define GPIO_BASE   (PERIPHERAL_BASE | 0x200000)
#define AUX_BASE    (PERIPHERAL_BASE | 0x215000)
#define TIMER_GEN_BASE  (PERIPHERAL_BASE | 0x3000)
#define TIMER_SYS_BASE  (PERIPHERAL_BASE | 0xB400)

#define UART0_BASE  (GPIO_BASE | 0x1000)

#define VC_MMU_BASE 0x40000000 // L2 caching enabled
// #define VC_MMU_BASE 0xC0000000 // L2 caching disabled

#define SYS_FREQ    250000000

// Memory-Mapped I/O
extern void mmio_write(uint32_t reg, uint32_t data);
extern uint32_t mmio_read(uint32_t reg);
#endif