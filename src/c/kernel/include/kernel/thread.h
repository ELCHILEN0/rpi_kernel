#include <pthread.h>

// TODO: 
int     kpthread_create(pthread_t *thread, void *(*start_routine)(void *), void *arg, enum process_priority priority);
// int     kpthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int     kpthread_equal(pthread_t thread_1, pthread_t thread_2);
void    kpthread_exit(void * status);
int     kpthread_join(pthread_t thread, void ** status);
pthread_t kpthread_self(void);