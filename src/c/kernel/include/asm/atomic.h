#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    int lock;
} spinlock_t;

void __spin_init(spinlock_t *lock);
void __spin_acquire(spinlock_t *lock);
void __spin_release(spinlock_t *lock);

#ifdef __cplusplus
};
#endif

#endif