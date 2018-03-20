#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "../asm/atomic.h"
#include "../sys/queue.h"

typedef struct usem {
  unsigned int id;  
} sem_t;

// TODO: Move to impl, this is not global
typedef struct ksem {
    spinlock_t lock;
    int count;
    TAILQ_HEAD(waiting, task) head;
} ksem_t;

int sem_init    (sem_t *sem, int pshared, unsigned int value);
int sem_destroy (sem_t *sem);
int sem_wait    (sem_t *sem);
int sem_trywait (sem_t *sem);
int sem_post    (sem_t *sem);
int sem_getvalue (sem_t *sem, int *sval);

int ksem_init    (sem_t *sem, int pshared, unsigned int value);
int ksem_destroy (sem_t *sem);
int ksem_wait    (sem_t *sem);
int ksem_trywait (sem_t *sem);
int ksem_post    (sem_t *sem);
int ksem_getvalue (sem_t *sem, int *sval);

#ifdef __cplusplus
};
#endif

#endif