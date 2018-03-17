#include "include/semaphore.h"

#include "include/kinit.h"

int __sem_init (sem_t *sem, int pshared, unsigned int value) {
    if (pshared != 0) {
        current->ret = ENOSYS;
        return OK;
    }

    sem->lock.flag = 0;
    sem->count = value;
    INIT_LIST_HEAD(&sem->tasks);

    current->ret = 0;
    return OK;
}

int __sem_destroy (sem_t *sem) {
    __spin_lock(&sem->lock);
    bool condition(process_t *curr) {
        return true;
    }
    alert_on_locked(&sem->tasks, condition);

    current->ret = 0;
    return OK;
}

int __sem_wait (sem_t *sem) {
    __spin_lock(&sem->lock);

    if (likely(sem->count > 0)) {
        sem->count--;
        __spin_unlock(&sem->lock); 

        current->ret = 0;
        return OK;
    } else {
        sleep_on_locked(&sem->tasks, current);
        __spin_unlock(&sem->lock);

        current->ret = 0;
        return BLOCK;
    }
}

int __sem_trywait (sem_t *sem) {
    __spin_lock(&sem->lock);

    int count = sem->count - 1;
    if (likely(count >= 0))
        sem->count = count;

    __spin_unlock(&sem->lock); 

    current->ret = count < 0 ? EAGAIN : 0;
    return OK;
}	

int __sem_post (sem_t *sem) {
    __spin_lock(&sem->lock);

    bool woke = false;
    bool condition(process_t *curr) {
        return woke == false;
    }

    if (likely(list_empty(&sem->tasks)))
        sem->count++;
    else
        alert_on_locked(&sem->tasks, condition);

    __spin_unlock(&sem->lock); 

    current->ret = 0;
    return OK;
}

int __sem_getvalue (sem_t *sem, int *sval) {
    __spin_lock(&sem->lock);

    *sval = sem->count;

    __spin_unlock(&sem->lock);

    current->ret = 0;
    return OK;
}
