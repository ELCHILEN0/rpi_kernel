#include "kernel.h"

int msb(uint64_t x) {
    if (!x) return -1;

    return __builtin_clz(x);
}

int lsb(uint64_t x) {
    return __builtin_ffs(x) - 1;
}