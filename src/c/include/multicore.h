#ifndef MULTICORE_H
#define MULTICORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t get_core_id( void );
extern void core_enable(uint32_t core, uint64_t addr);

typedef uint8_t lock_t;

typedef volatile struct {
    uint8_t flag;
} spinlock_t;

// typedef volatile struct {
//     lock_t lock;
//     uint64_t count;
//     TODO: concept of a queue of waiters, or a single blocked process... need to identify this
//     and keep into account the fact that this might be called from the kernel..., only wait from kernel....
// } semaphore_t;

extern int __spin_lock(spinlock_t *lock);
extern void __spin_unlock(spinlock_t *lock);

#ifdef __cplusplus
};
#endif

#endif