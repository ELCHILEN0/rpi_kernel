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


extern void timer_init( void );
extern unsigned int timer_read( void );
extern void timer_wait(unsigned int wait);