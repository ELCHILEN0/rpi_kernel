#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    volatile uint32_t control_status;
    volatile uint32_t counter_lo;
    volatile uint32_t counter_hi;
    volatile uint32_t compare0;
    volatile uint32_t compare1;
    volatile uint32_t compare2;
    volatile uint32_t compare3;
} core_timer_t;

/**
 * The ARM 64 bit Core Timer is core related interrupt source, meaning the signal
 * may be sent to EVERY core.
 */
typedef enum {
    CT_CTRL_INC1 =      (0 << 0),
    CT_CTRL_INC2 =      (1 << 9),
    CT_CTRL_SRC_CRY =   (0 << 0),   // Quartz Clock Source
    CT_CTRL_SRC_APB =   (1 << 8),   // 1/2 ARM CPU Frequency
} core64_timer_control_t;

typedef enum {
    CT_FIQ_VIRT =       0x40,
    CT_FIQ_HYP  =       0x30,
    CT_FIQ_NON_SECURE = 0x20,
    CT_FIQ_SECURE =     0x10,
    CT_IRQ_VIRT =       0x4,
    CT_IRQ_HYP =        0x3,
    CT_IRQ_NON_SECURE = 0x2,
    CT_IRQ_SECURE =     0x1,
} core64_timer_interrupt_t;

typedef volatile struct {
    uint32_t control;
    uint32_t _prescaler_subtract; // DEPRECATED
    uint32_t prescaler;
    uint32_t _RESERVED_1[(0x1C - 0x0C) / sizeof(uint32_t)];

    /**
     * The lower bits must always be read before the upper bits to ensure the
     * measured time is accurate.
     */
    uint32_t low; 
    uint32_t high;

    uint32_t _RESERVED_2[(0x40 - 0x24) / sizeof(uint32_t)];
    uint32_t interrupt_control[4];
} core64_timer_t;

/**
 * The ARM Local Timer is a core un-related interrupt source, meaning the signal
 * can be sent to any ONE core.
 */
typedef volatile struct {
    uint32_t interrupt_routing;
    uint32_t _RESERVED_0[(0x34 - 0x24) / sizeof(uint32_t) - 1];
    uint32_t control_status;
    uint32_t irq_clear_reload;
} local_timer_t;

extern void local_timer_interrupt_routing( unsigned int mode );
extern void local_timer_start( unsigned int reload );
extern void local_timer_reset( void );

extern void core_timer_init( core64_timer_control_t src, core64_timer_control_t inc, uint32_t prescaler );
extern void core_timer_interrupt_routing( uint8_t core_id, core64_timer_interrupt_t type );
extern void core_timer_rearm( uint64_t ticks );
extern void core_timer_stop();
extern uint64_t core_timer_count();

local_timer_t *local_timer;
core64_timer_t *core64_timer;

#endif