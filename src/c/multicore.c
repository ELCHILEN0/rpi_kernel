#include "multicore.h"

#include <stdio.h>
#include "peripheral.h"

uint32_t get_core_id( void ) {
    uint32_t core_id;
    asm("mrc     p15, 0, %0, c0, c0, 5; \
        ubfx    %0, %0, #0, #2" : "=r" (core_id));

    return core_id;
}


// int test_and_set(volatile int *addr) {
//     int old_value = swap_atomic(addr, 1);

//     if (old_value == 0)
//         return 0;

//     return 1;
// }

// 18.5.2
int __spin_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(&lock->flag, 1));
    return 0;
}

void __spin_unlock(spinlock_t *lock) {
    __sync_lock_release(&lock->flag);
}

#define CORE0_MBOX3_SET             0x4000008C
#define CORE1_MBOX3_R               0x400000DC
#define CORE2_MBOX3_R               0x400000EC
#define CORE3_MBOX3_R               0x400000FC

void core_enable(uint32_t core, uint32_t addr)
{
    mmio_write(CORE0_MBOX3_SET + 0x10 * core, addr);

    // printf("[core] Enabling 0x%X -> (0x%X)\r\n", CORE0_MBOX3_SET + 0x10 * core, addr);
    // printf("[core1] Value @ 0x%X = (0x%X)\r\n", CORE1_MBOX3_R, mmio_read(CORE1_MBOX3_R));
    // printf("[core2] Value @ 0x%X = (0x%X)\r\n", CORE1_MBOX3_R, mmio_read(CORE2_MBOX3_R));
    // printf("[core3] Value @ 0x%X = (0x%X)\r\n", CORE1_MBOX3_R, mmio_read(CORE3_MBOX3_R));
    // printf("--------\r\n");
    asm("SEV");

}
