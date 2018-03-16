#include "kinit.h"

int msb(uint64_t x) {
    if (!x) return -1;

    return 32 - __builtin_clz(x); // TODO: Can 32 be derived...
}

int lsb(uint64_t x) {
    return __builtin_ffs(x) - 1;
}