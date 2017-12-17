#include <stdint.h>
#include <stdio.h>

#include "aux.h"
#include "timer.h"

#define PMAP_BASE 0x3F000000
#define TIMER_OFFSET 0x0000B400
#define TIMER_SYS_OFFSET 0x00003000
#define TIMER_BASE (PMAP_BASE|TIMER_OFFSET)
#define TIMER_SYS_BASE (PMAP_BASE|TIMER_SYS_OFFSET)

#define SYSTIMERCLO 0x3F003004
#define GPFSEL3     0x3F20000C
#define GPFSEL4     0x3F200010
#define GPSET1      0x3F200020
#define GPCLR1      0x3F20002C

#define TIMER_BIT 0x00400000

// typedef struct {
//     volatile uint32_t control_status;
//     volatile uint32_t counter_lo;
//     volatile uint32_t counter_hi;
//     volatile uint32_t compare0;
//     volatile uint32_t compare1;
//     volatile uint32_t compare2;
//     volatile uint32_t compare3;
// } timer_t;


void timer_init( void ) {
    unsigned int ra;

    ra=mmio_read(GPFSEL4);
    ra&=~(7<<21);
    ra|=1<<21;
    mmio_write(GPFSEL4,ra);

    ra=mmio_read(GPFSEL3);
    ra&=~(7<<15);
    ra|=1<<15;
    mmio_write(GPFSEL3,ra);


    while(1)
    {
        mmio_write(GPSET1,1<<(47-32));
        mmio_write(GPCLR1,1<<(35-32));
        while(1)
        {
            ra=mmio_read(SYSTIMERCLO);
            // printf("%d, %d\n", ra, ra & TIMER_BIT);
            if((ra&=TIMER_BIT)==TIMER_BIT) break;
        }
        mmio_write(GPCLR1,1<<(47-32));
        mmio_write(GPSET1,1<<(35-32));
        while(1)
        {
            ra=mmio_read(SYSTIMERCLO);
            if((ra&=TIMER_BIT)==0) break;
        }
    }
}
unsigned int timer_read( void ) {

}

void timer_wait(unsigned int wait)
{

}