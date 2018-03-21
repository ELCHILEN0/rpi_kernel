#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int lock;
} spinlock_t;

void __spin_init(spinlock_t *lock)
{
    lock->lock = 0;
}

void __spin_acquire(spinlock_t *lock)
{
    while (__sync_lock_test_and_set(&lock->lock, 1));
}

void __spin_release(spinlock_t *lock)
{
    __sync_lock_release(&lock->lock);    
}

#ifdef __cplusplus
};
#endif

#endif