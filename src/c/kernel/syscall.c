#include "kernel.h"

/* Your code goes here */
int syscall( int req_id, ... ) {
    int ret_code;
 
    va_list args;
    va_start(args, req_id);

    asm volatile("STP %0, %1, [SP, #-16]!" :: "r" (req_id), "r" (&args));
    asm volatile("SVC 0x80");
    asm volatile("ADD SP, SP, #16");
    asm volatile("MOV %0, X0" : "=g" (ret_code));

    va_end(args);

    return ret_code;
}

pid_t syscreate( void *(*start_routine)(void *), void *arg ) {
    return syscall(SYS_CREATE, start_routine, arg);
}

void sysyield( void ) {
    syscall(SYS_YIELD);
}

void sysexit( void ) {
    syscall(SYS_EXIT);
}

uint64_t syswaitpid( pid_t pid ) {
    return syscall(SYS_WAIT_PID, pid);
}

pid_t sysgetpid( void ) {
    return syscall(SYS_GET_PID);
}

int syskill( pid_t pid, int sig ) {
    return syscall(SYS_KILL, pid, sig);
}

uint64_t syssleep(unsigned int ms) {
    return syscall(SYS_SLEEP, ms);
}

// Scheduler API ...
int sched_yield(void)
{
    return syscall(SYS_YIELD);
}

// int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask) {
//     return syscall(SCHED_SET_AFFINITY, pid, cpusetsize, mask);
// }

// POSIX Thread API ...
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    return syscall(PTHREAD_CREATE, thread, attr, start_routine, arg);
}

int pthread_equal(pthread_t thread_1, pthread_t thread_2) {
    return thread_1 == thread_2;
}

void pthread_exit(void * status) {
    syscall(PTHREAD_EXIT, status);
}

int pthread_join(pthread_t thread, void ** status) {
    return syscall(PTHREAD_JOIN, thread, status);
}

pthread_t pthread_self(void) {
    return syscall(PTHREAD_SELF);
}

// int pthread_mutex_init(pthread_mutex_t * mutex,
//         const pthread_mutex_attr *attr);

// int pthread_mutex_destroy(pthread_mutex_t * mutex);

// int pthread_mutex_lock(pthread_mutex_t * mutex);

// int pthread_mutex_trylock(pthread_mutex_t * mutex);

// int pthread_mutex_unlock(pthread_mutex_t * mutex);
