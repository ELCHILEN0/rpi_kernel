#include "multicore.h"

#include <stdio.h>
#include "peripheral.h"

uint32_t get_core_id( void ) {
    uint32_t core_id;
    asm("mrc p15, 0, %0, c0, c0,  5" : "=r" (core_id));
    return core_id & 0b11;
}

#define CORE0_MBOX3_SET             0x4000008C
#define CORE1_MBOX3_R               0x400000DC

void core_enable(uint32_t core, uint32_t addr)
{
    printf("[core] Enabling 0x%X -> (0x%X)\r\n", CORE0_MBOX3_SET + 0x10 * core, addr);
    mmio_write(CORE0_MBOX3_SET + 0x10 * core, addr);
    // *(uint32_t*)(0x4000008C + 0x10 * core) = addr;

    printf("[core] Value @ 0x%X = (0x%X)\r\n", CORE1_MBOX3_R, mmio_read(CORE1_MBOX3_R));
    asm("SEV");
    // volatile uint32_t *p;
    // p = (uint32_t*)(CORE0_MBOX3_SET + 0x10 * core);
     
    // *p = addr;
}
