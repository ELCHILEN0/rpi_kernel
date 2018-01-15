#include <stdint.h>

typedef struct {
    volatile uint32_t control_status;
    volatile uint32_t counter_lo;
    volatile uint32_t counter_hi;
    volatile uint32_t compare0;
    volatile uint32_t compare1;
    volatile uint32_t compare2;
    volatile uint32_t compare3;
} timer_gen_t;


typedef volatile struct {
    uint32_t interrupt_routing;
    uint32_t _RESERVED_0[(0x34 - 0x24) / sizeof(uint32_t) - 1];
    uint32_t control_status;
    uint32_t irq_clear_reload;
} local_timer_t;

extern void timer_init( unsigned int reload );
extern void timer_reset( unsigned int reload );
extern unsigned int timer_read( void );
extern void timer_wait(unsigned int wait);

local_timer_t *local_timer;