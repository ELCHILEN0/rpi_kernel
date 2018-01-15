#include <stdint.h>
#include <stdio.h>

#include "timer.h"
#include "peripheral.h"

local_timer_t *local_timer = (local_timer_t *) 0x40000024;

extern void timer_reset( unsigned int reload ) {
    local_timer->control_status &= ~(1 << 29);
    local_timer->control_status &= ~(1 << 28);
    local_timer->irq_clear_reload = 1 << 31;

    local_timer->control_status |= 1 << 28;  
    local_timer->control_status |= 1 << 29; 
    local_timer->control_status |= reload; 
}

extern void timer_init( unsigned int reload ) {
    local_timer->interrupt_routing = 0x000; // route to core 0

    local_timer->control_status |= 1 << 29;
    local_timer->control_status |= 1 << 28;
    local_timer->irq_clear_reload &= 0xF0000000;
    local_timer->irq_clear_reload |= reload;
}

extern unsigned int timer_read( void ) {

}

extern void timer_wait(unsigned int wait)
{

}