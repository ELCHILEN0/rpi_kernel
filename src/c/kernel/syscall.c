/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */
int syscall( int req_id, ... ) {
    int ret_code;

    // Start building the list of arguments passed to the syscall function
    // and pass the pointer of the va_list to the dispatcher.   
    va_list args;
    va_start(args, req_id);


    /* Invoking a system call, switching to the kernel
     * Caller:
     * 1.  Push the arguments in reverse order
     * Prolog: 
     * 2.  Save and update ebp
     * 3.  Transfer control to the kernel
     * Epilog:
     * 4.  Restore esp and ebp
     * 5.  Move return value to eax
     * 
     * https://www.cs.princeton.edu/courses/archive/spr11/cos217/lectures/15AssemblyFunctions.pdf
     */
    __asm __volatile("\
        pushl %2	\n\
        pushl %1	\n\
        pushl %%ebp	\n\
	movl %%esp, %%ebp	\n\
        int %3	\n\
	movl %%ebp, %%esp	\n\
	popl %%ebp	\n\
        movl %%eax, %0	\n\
    "
    : "=g" (ret_code)
    : "g" (req_id), "g" (&args), "g" (KERNEL_INT)
    : "%eax"
    );

    va_end(args);

    return ret_code;
}

unsigned int syscreate( void (*func)(void), int stack ) {
    return syscall(SYS_CREATE, func, stack);
}

void sysyield( void ) {
    syscall(SYS_YIELD);
}
void sysstop( void ) {
    syscall(SYS_STOP);
}

pid_t sysgetpid( void ) {
    return syscall(SYS_GET_PID);
}

int syskill( pid_t pid, int sig ) {
    return syscall(SYS_KILL, pid, sig);
}

void sysputs( char *str ) {
    syscall(SYS_PUTS, str);
}

unsigned int syssleep( unsigned int ms ) {
    return syscall(SYS_SLEEP, ms);
}

int syssend( pid_t pid, void *buffer, int len ) {
    return syscall(SYS_SEND, pid, buffer, len);
}

int sysrecv( pid_t *pid, void *buffer, int len ) {
    return syscall(SYS_RECV, pid, buffer, len);
}

int sysgetcputimes( processStatuses *ps ) {
    return syscall(SYS_CPUTIMES, ps);
}

void syssigreturn( void *old_stack_frame ) {
    syscall(SYS_SIG_RET, old_stack_frame);
}

int syssighandler( int sig, void (*new_handler)(void*), void (**old_handler)(void*) ) {
    return syscall(SYS_SIG, sig, new_handler, old_handler); 
}

int syswait( pid_t pid ) {
    return syscall(SYS_WAIT, pid);
}

int sysopen( int device_no ) {
    return syscall(SYS_OPEN, device_no);
}

int sysclose( int fd ) {
    return syscall(SYS_CLOSE, fd);
}

int syswrite( int fd, void *buff, int len ) {
    return syscall(SYS_WRITE, fd, buff, len);
}

int sysread( int fd, void *buff, int len ) {
    return syscall(SYS_READ, fd, buff, len);
}

int sysioctl( int fd, unsigned long command, ... ) {
    va_list args;
    va_start(args, command);
    
    int ret_code = syscall(SYS_IOCTL, fd, command, args);

    va_end(args);
    return ret_code;
}

