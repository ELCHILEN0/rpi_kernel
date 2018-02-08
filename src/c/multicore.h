#ifndef MULTICORE_H
#define MULTICORE_H

#include <stdint.h>

extern uint32_t get_core_id( void );
extern void core_enable(uint32_t core, uint64_t addr);

typedef int lock_t;

typedef volatile struct {
    uint8_t flag;
} spinlock_t;

extern int __spin_lock(spinlock_t *lock);
extern void __spin_unlock(spinlock_t *lock);
#endif