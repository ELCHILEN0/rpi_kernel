#ifndef SCHED_H
#define SCHED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sched.h>

typedef struct cpu_set {
    uint8_t mask;
} cpu_set_t;

void CPU_ZERO   (cpu_set_t *set);
void CPU_SET    (int cpu, cpu_set_t *set);
void CPU_CLR    (int cpu, cpu_set_t *set);
int  CPU_ISSET  (int cpu, cpu_set_t *set);
int  CPU_COUNT  (cpu_set_t *set);

int sched_yield (void);
int sched_set_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);
int sched_get_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);

int ksched_yield(void);
int ksched_set_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);
int ksched_get_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);

struct context *current (void);
struct context *runnable(void);

void set_current    (struct context *curr);
void set_runnable   (struct context *curr);

#ifdef __cplusplus
};
#endif

#endif