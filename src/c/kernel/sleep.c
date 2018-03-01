#include "kernel.h"

struct list_head sleep_queue[NUM_CORES]; // Each core has its own sleep queue, avoids conflicting ticks.

uint64_t ms_to_ticks(uint64_t ms) {
    return ms / (1000 / CLOCK_DIVD);
}

enum syscall_return_state proc_sleep(process_t *proc, unsigned int ms) {
    uint8_t core_id = get_core_id();   

    // Ticks to sleep
    proc->tick_delta = ms_to_ticks(ms);
    if (proc->tick_delta <= 0)
        return OK;
    
    // Find dlist position
    struct list_head *cursor;
    list_for_each(cursor, &sleep_queue[core_id]) {
        process_t *entry = list_entry(cursor, process_t, block_list);

        if (proc->tick_delta <= entry->tick_delta)
            break;

        proc->tick_delta -= entry->tick_delta;
    }

    // Add to dlist
    list_add_tail(&proc->block_list, cursor);

    // Adjust next offset
    if (list_has_next(&proc->block_list, &sleep_queue[core_id])) {
        process_t *entry = list_entry(proc->block_list.next, process_t, block_list);
        entry->tick_delta -= proc->tick_delta;
    }

    return BLOCK;
}

void global_tick() {
    uint8_t core_id = get_core_id();

    if (list_empty(&sleep_queue[core_id])) return;

    // O(1) decrement first list.head
    process_t *next, *process = list_entry(sleep_queue[core_id].next, process_t, block_list);
    process->tick_delta--;

    // Iterate from list.head waking available entries
    list_for_each_entry_safe(process, next, &sleep_queue[core_id], block_list) {
        if (process->tick_delta > 0) 
            break;
   
        ready(process);
        process->ret = 0;
    }
}

enum syscall_return_state proc_tick(process_t *proc) {
    proc->tick_count++;

    global_tick();

    return SCHED;
}