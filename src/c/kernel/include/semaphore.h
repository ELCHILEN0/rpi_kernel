#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <errno.h>

#include "config.h"

#include "list.h"
#include "sched.h"
#include "context.h"

int sem_init    (sem_t *sem, int pshared, unsigned int value);
int sem_destroy (sem_t *sem);
int sem_wait    (sem_t *sem);
int sem_trywait (sem_t *sem);	
int sem_post    (sem_t *sem);	
int sem_getvalue (sem_t *sem, int *sval);

int __sem_init    (sem_t *sem, int pshared, unsigned int value);
int __sem_destroy (sem_t *sem);
int __sem_wait    (sem_t *sem);
int __sem_trywait (sem_t *sem);	
int __sem_post    (sem_t *sem);	
int __sem_getvalue (sem_t *sem, int *sval);


#ifdef __cplusplus
};
#endif

#endif