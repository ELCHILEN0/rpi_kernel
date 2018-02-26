#include "kernel.h"

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

pid_t syscreate( void (*func)(void), uint64_t stack ) {
    return syscall(SYS_CREATE, func, stack);
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

void syssigreturn(void *frame) {
    return syscall(SYS_SIG_RET, frame);
}