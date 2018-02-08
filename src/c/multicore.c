#include "multicore.h"

#include <stdio.h>
#include "peripheral.h"

uint32_t get_core_id( void ) {
    uint32_t core_id;

    asm ("MRS x0, MPIDR_EL1     \n\
          UBFX x0, x0, #0, #2   \n\
    " : "=r" (core_id));

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
    // while (__sync_lock_test_and_set(&lock->flag, 1));
    return 0;
}

void __spin_unlock(spinlock_t *lock) {
    // __sync_lock_release(&lock->flag);
}

/*
 * Signal the specified core to jump to the address at addr.  After signaling,
 * the calling core goes into WFE mode until the target core signals SEV.  This
 * is to ensure startup code can run.
 * 
 * https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S#L109
 */
static uint64_t *cpu_release_addr = (uint64_t *) 0xd8;

void core_enable(uint32_t core, uint64_t addr)
{
    cpu_release_addr[core] = addr;

    asm("SEV");
}

void core_sleep() {
    uint32_t core_id = get_core_id();
    uint64_t wakeup_addr = 0;
    cpu_release_addr[core_id] = wakeup_addr;    
    while ((wakeup_addr = cpu_release_addr[core_id]) == 0) {
        asm("WFE");
    }
    asm("BR %0" :: "r" (wakeup_addr));
}