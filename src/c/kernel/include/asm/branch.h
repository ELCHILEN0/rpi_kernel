#ifndef BRANCH_H
#define BRANCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#ifdef __cplusplus
};
#endif

#endif