#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

#include "aux.h"

int _close(int file) { return -1; }

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) { return 1; }

int _lseek(int file, int ptr, int dir) { return 0; }

int _open(const char *name, int flags, int mode) { return -1; }

int _read(int file, char *ptr, int len) {
    int i;
    for (int i = 0; i < len; i++) {
        ptr[i] = uart_getc();
    }

    return i;
}

caddr_t _sbrk(int incr) {
    extern char *__heap_start;
    extern char *__heap_end;
    static char *heap_end = NULL;
    char *prev_heap_end;
    
    if (heap_end == NULL) {
        heap_end = &__heap_start;
    }
    prev_heap_end = heap_end;
    
    if (heap_end + incr > &__heap_end) {
        return (caddr_t) 0;
    }
    
    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

int _write(int file, char *ptr, int len) {
    int i;
    
    for (i = 0; i < len; i++) {
        uart_putc(ptr[i]);
    }
    
    return len;
}