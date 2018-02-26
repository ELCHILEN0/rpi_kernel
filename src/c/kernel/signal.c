#include "kernel.h"

int msb(uint64_t x) {
    if (!x) return -1;

    return 32 - __builtin_clz(x); // TODO: is 32 right...
}

int lsb(uint64_t x) {
    return __builtin_ffs(x) - 1;
}

int signal(pid_t pid, int sig) {
    // TODO: expand on this, error handling
}