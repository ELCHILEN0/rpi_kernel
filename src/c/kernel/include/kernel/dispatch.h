#ifndef DISPATCH_H
#define DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

enum interrupt_source {
    INT_SYSCALL,
    INT_TIMER,
};

enum interrupt_request {
    SYS_TIME_SLICE,

    SYS_KILL,
    SYS_SLEEP,
    SYS_SET_PERF,
    SYS_GET_PERF,
    SYS_PUTS,
    SYS_GETS,   

    SCHED_YIELD,
    SCHED_SET_AFFINITY,
    SCHED_GET_AFFINITY,

    PTHREAD_CREATE,
    PTHREAD_EQUAL,
    PTHREAD_EXIT,
    PTHREAD_JOIN,
    PTHREAD_SELF,
    PTHREAD_MUTEX_INIT,
    PTHREAD_MUTEX_DESTROY,
    PTHREAD_MUTEX_LOCK,
    PTHREAD_MUTEX_TRYLOCK,
    PTHREAD_MUTEX_UNLOCK,

    SEM_INIT,
    SEM_DESTROY,
    SEM_WAIT,
    SEM_TRY_WAIT,
    SEM_POST,
    SEM_GET_VALUE,
};

void disp_init();

void timer_interrupt();
void scall_interrupt();

void common_interrupt( int interrupt_type );
void dispatch_request( int request );

#ifdef __cplusplus
};
#endif

#endif