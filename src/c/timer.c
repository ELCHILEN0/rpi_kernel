#include <stdint.h>
#include <stdio.h>

#include "timer.h"
#include "peripheral.h"

local_timer_t *local_timer = (local_timer_t *) 0x40000024;

extern void local_timer_reset( void ) {
    local_timer->irq_clear_reload = 1 << 31;
}

extern void local_timer_start( unsigned int reload ) {
    local_timer->control_status = reload & 0x0FFFFFFF;

    local_timer->control_status |= 1 << 29;
    local_timer->control_status |= 1 << 28;
}

extern void local_timer_interrupt_routing( unsigned int mode ) {
    local_timer->interrupt_routing = mode;
}