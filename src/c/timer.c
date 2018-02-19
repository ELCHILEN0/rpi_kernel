#include "timer.h"
#include "peripheral.h"

local_timer_t *local_timer = (local_timer_t *) 0x40000024;
core64_timer_t *core64_timer = (core64_timer_t *) 0x40000000;

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

extern void core_timer_init( core64_timer_control_t src, core64_timer_control_t inc, uint32_t prescaler ) {
    core64_timer->control = src | inc;
    core64_timer->prescaler = prescaler;
}

extern void core_timer_interrupt_routing(uint8_t core_id, core64_timer_interrupt_t type) {
    core64_timer->interrupt_control[core_id] |= type;    
}

extern void core_timer_rearm(uint64_t ticks) {
    // On a write to CNTP_TVAL, CNTP_CVAL_EL1 is set to (CNTPCT_EL0 + TimerValue)
    asm volatile("MSR CNTP_CTL_EL0, %0" :: "r" (0));
    asm volatile("MSR CNTP_TVAL_EL0, %0" :: "r" (ticks));    
    asm volatile("MSR CNTP_CTL_EL0, %0" :: "r" (1)); 
}

extern void core_timer_stop() {
    asm volatile("MSR CNTP_CTL_EL0, %0" :: "r" (0));
}
