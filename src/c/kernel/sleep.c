#include "include/kinit.h"

struct list_head sleep_queue[NUM_CORES]; // Each core has its own sleep queue, avoids conflicting ticks.

uint64_t ms_to_ticks(uint64_t ms) {
    return ms / (1000 / CLOCK_DIVD);
}

enum return_state proc_sleep(unsigned int ms) {
    // Ticks to sleep
    current->tick_delta = ms_to_ticks(ms);
    if (current->tick_delta <= 0)
        return OK;
    
    struct list_head *head = &sleep_queue[get_core_id()];

    process_t *entry;
    // Find insert position + delta offset
    list_for_each_entry(entry, head, sched_list) {
        if (current->tick_delta <= entry->tick_delta)      
            break;

        current->tick_delta -= entry->tick_delta;
    }

    // Add to dlist
    if (list_empty(head)) {
        list_add(&current->sched_list, head);
    } else {
        list_add(&current->sched_list, &entry->sched_list);

        // Decrement the next entry
        list_for_each_entry_continue(current, head, sched_list) {
            entry->tick_delta -= current->tick_delta;
            break;     
        }
    }
    
    return BLOCK;
}

void proc_wake(process_t *proc) {

}

void global_tick() {
    uint8_t core_id = get_core_id();

    struct list_head *head = &sleep_queue[core_id];

    // O(1) decrement first list.head
    process_t *process, *next;
    list_for_each_entry_safe(process, next, head, sched_list) {
        if (--process->tick_delta > 0)
            break;

        list_del_init(&process->sched_list);

        ready(process);
        process->ret = 0;
    }
}

enum return_state proc_tick() {
    current->tick_count++;

    // global_tick();

    return SCHED;
}