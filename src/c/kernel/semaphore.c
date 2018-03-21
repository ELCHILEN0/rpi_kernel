#include "include/kernel.h"

ksem_t *ksem_open (sem_t *sem) {
    return NULL;
}

int ksem_init (sem_t *sem, int pshared, unsigned int value) {
    if (pshared != 0) {
        return ENOSYS;
    }

    ksem_t *ksem = ksem_open(sem);
    __spin_init(&ksem->lock);
    TAILQ_INIT(&ksem->head);
    ksem->count = value;

    return 0;
}

int ksem_destroy (sem_t *sem) {
    ksem_t *ksem = ksem_open(sem);

    __spin_acquire(&ksem->lock);
    sleepq_alert_locked(&ksem->head, sleepq_condition_true);

    return 0;
}

int ksem_wait (sem_t *sem) {
    ksem_t *ksem = ksem_open(sem);    

    __spin_acquire(&ksem->lock);

    if (likely(ksem->count > 0)) {
        ksem->count--;

        current()->state = OK;
    } else {
        sleepq_add_locked(&ksem->head, current);

        current()->state = BLOCK;
    }

    __spin_release(&ksem->lock);    

    return 0;
}

int ksem_trywait (sem_t *sem) {
    ksem_t *ksem = ksem_open(sem);    
    
    __spin_acquire(&ksem->lock);

    int count = ksem->count - 1;
    if (likely(count >= 0))
        ksem->count = count;

    __spin_release(&ksem->lock); 

    return count < 0 ? EAGAIN : 0;
}	

int ksem_post (sem_t *sem) {
    ksem_t *ksem = ksem_open(sem);    
    
    __spin_acquire(&ksem->lock);

    if (likely(TAILQ_EMPTY(&ksem->head)))
        ksem->count++;
    else
        sleepq_alert_locked_nr(&ksem->head, sleepq_condition_true, 1);

    __spin_release(&ksem->lock); 

    return 0;
}

int ksem_getvalue (sem_t *sem, int *sval) {
    ksem_t *ksem = ksem_open(sem);    
    
    __spin_acquire(&ksem->lock);

    *sval = ksem->count;

    __spin_release(&ksem->lock);

    return 0;
}
