#include <stdint.h>

#define PERIPHERAL_BASE 0x3F000000

#define GPIO_BASE   (PERIPHERAL_BASE | 0x200000)
#define TIMER_GEN_BASE  (PERIPHERAL_BASE | 0x3000)
#define TIMER_SYS_BASE  (PERIPHERAL_BASE | 0xB400)

#define UART0_BASE  (GPIO_BASE | 0x1000)

// Memory-Mapped I/O
extern void mmio_write(uint32_t reg, uint32_t data);
extern uint32_t mmio_read(uint32_t reg);