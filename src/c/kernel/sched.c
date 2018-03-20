#include "include/kernel/sched.h"
#include "include/kernel/const.h"
#include "include/kernel/context.h"
#include "include/kernel/globals.h"
#include "include/asm/cpu.h"

void CPU_ZERO(cpu_set_t *set) {
    set->mask = 0;
}
void CPU_SET(int cpu, cpu_set_t *set) {
    set->mask |= (1 << cpu);
}
void CPU_CLR(int cpu, cpu_set_t *set) {
    set->mask &= ~(1 << cpu);
}
int  CPU_ISSET(int cpu, cpu_set_t *set) {
    return set->mask & (1 << cpu);
}
int  CPU_COUNT(cpu_set_t *set) {
    int count;
    for (int cpu = 0; cpu < NUM_CORES; cpu++) {
        if (CPU_ISSET(cpu, set))
            count++;
    }
    return count;
}

void set_current    (struct context *curr) {
    running[cpu_id()] = curr;
}

struct context *current (void) {
    return running[cpu_id()];
}

int ksched_yield(void) {
    current()->state = SCHED;
    return 0;
}

int ksched_set_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask) {
    current()->affinity.mask = mask->mask;
    return 0;
}

int ksched_get_affinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask) {
    mask->mask = current()->affinity.mask;
    return 0;
}

int find_busy_core() {
    return 0;
}

int find_idle_core(cpu_set_t *mask) {
    int idle_core = 0;
    int idle_count = INT32_MAX;

    for (int cpu = 0; cpu < NUM_CORES; cpu++) {
        ready_queue_t *queue = &ready_queue[cpu];

        if (queue->length < idle_count && CPU_ISSET(cpu, mask))
        {
            idle_core = cpu;
            idle_count = queue->length;
        }
    }
 }

void set_runnable   (struct context *curr) {
    curr->state = RUNNABLE;

    #ifdef SCHED_AFFINITY
    int idle_core = find_idle_core(&curr->affinity);
    ready_queue_t *target_queue = &ready_queue[get_core_id()];

    __spin_acquire(&target_queue->lock);

    // Migration to another core will happen under specific conditions:
    // - The current core is not eligible to run the process (cpu_set_t)
    // - The current core has more than 25 % of all the live processes
    // TODO: Cache affinity, metric to "encourage" processes to remain
    // TODO: Work stealing at sporadic intervals
    if (!CPU_ISSET(get_core_id(), &curr->affinity)
            || (100 * target_queue->length / live_tasks) > 25) 
    {
        __spin_release(&target_queue->lock);
        target_queue = &ready_queue[idle_core];
        __spin_acquire(&target_queue->lock); 
    }
    
    target_queue->length += 1;
    TAILQ_INSERT_TAIL(target_queue->head[curr->current_priority], curr, sched_list);
    __spin_release(&target_queue->lock);
        
    #else
    __spin_acquire(&scheduler_lock);    
    list_del_init(&process->sched_list);    
    list_add_tail(&process->sched_list, &ready_queue[process->current_priority]);
    __spin_release(&scheduler_lock);        
    #endif
}

struct context *runnable (void) {
    for (int i = PRIORITY_HIGH; i >= PRIORITY_IDLE; i--) {
        #ifdef SCHED_AFFINITY        
            ready_queue_t *target_queue = &ready_queue[cpu_id()];

            __spin_acquire(&target_queue->lock);
            struct task_list *head = target_queue->head[i];
        #else
            __spin_acquire(&scheduler_lock);        
            struct task_list *head = &ready_queue[i];
        #endif     

        task_t *task = NULL;
        if (!TAILQ_EMPTY(head)) {
            task = TAILQ_FIRST(head);
            TAILQ_REMOVE(head, task, sched_list);
        }

        #ifdef SCHED_AFFINITY
            target_queue->length -= 1;
            __spin_release(&target_queue->lock);    
        #else        
            __spin_release(&scheduler_lock);
        #endif

        if (task)
            return task;
    }

    return NULL; // TODO: while(true); ... should never run out of processes
}