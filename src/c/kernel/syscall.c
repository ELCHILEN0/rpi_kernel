/* syscall.c : syscalls
 */

#include "kernel.h"
#include <stdarg.h>

/* Your code goes here */
int syscall( int req_id, ... ) {
    int ret_code;
 
    va_list args;
    va_start(args, req_id);

    asm volatile("MOV r1, %0" :: "r" (req_id));
    asm volatile("MOV r2, %0" :: "r" (args));
    asm volatile("SVC 0x80");
    asm volatile("MOV %0, r0" : "=r" (ret_code));

    va_end(args);

    return ret_code;
}

pid_t syscreate( void (*func)(void), uint32_t stack ) {
    return syscall(SYS_CREATE, func, stack);
}

void sysyield( void ) {
    syscall(SYS_YIELD);
}

void sysexit( void ) {
    syscall(SYS_EXIT);
}

int syswaitpid( pid_t pid ) {
    return syscall(SYS_WAIT_PID, pid);
}

pid_t sysgetpid( void ) {
    return syscall(SYS_GET_PID);
}

int syskill( pid_t pid, int sig ) {
    return syscall(SYS_KILL, pid, sig);
}