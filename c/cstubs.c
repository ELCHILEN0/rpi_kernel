#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "aux.h"

int uart_write( void *reent, int fd, const char *buf, int len ) {
    for (size_t i = 0; i < len; i++) {
	    uart_putc(buf[i]);
    }

    return len;
}

int uart_read( void *reent, int fd, char *buf, int len ) {
    for (size_t i = 0; i < len; i++) 
		buf[i] = uart_getc();

    return len;
}

typedef struct {
    const char *name;
    int (*open_r  )( void *reent, const char *path,int flags, int mode );
    int (*close_r )( void *reent, int fd );
    int (*write_r )( void *reent, int fd, const char *ptreent, int len );
    int (*read_r  )( void *reent, int fd, char *ptreent, int len );
} devoptab_t;

// TODO: Fix incompatible pointer warnings
const devoptab_t devoptab_uart = { 
    "uart",
    NULL,
    NULL,
    uart_write,
    uart_read
};

// TODO: Add more devices
const devoptab_t *devoptab_list[] = {
    &devoptab_uart, // stdin
    &devoptab_uart, // stdout
    &devoptab_uart, // stederr
    0
};

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) { return 1; }

int _lseek(int file, int ptr, int dir) { return 0; }

int _open(const char *name, int flags, int mode) { return -1; }

int _write(int fd, char *buf, int nbyte) {
    // TODO: Check fd in range
    const devoptab_t *device = devoptab_list[fd];
    if (device->write_r == NULL) return -1;

    return device->write_r( NULL, fd, buf, nbyte );
}

int _read(int fd, char *buf, int nbyte) {
    // TODO: Check fd in range
    const devoptab_t *device = devoptab_list[fd];
    if (device->read_r == NULL) return -1;

    return device->read_r( NULL, fd, buf, nbyte );
}

int _close(int fd) {
    // TODO: Check fd in range
    const devoptab_t *device = devoptab_list[fd];
    if (device->close_r == NULL) return -1;

    return device->close_r( NULL, fd );
}

caddr_t _sbrk(int incr) {
    extern char *__heap_start;
    extern char *__heap_end;

    static char *heap_end = NULL;
    char *prev_heap_end;
    
    if (heap_end == NULL) {
        heap_end = (char *) &__heap_start;
    }
    prev_heap_end = heap_end;
    
    if (heap_end + incr > (char *) &__heap_end) {
        return (caddr_t) 0;
    }
    
    heap_end += incr;
    return (caddr_t) prev_heap_end;
}