#include "kernel.h"

struct list_head sleep_queue[NUM_CORES]; // Each core has its own sleep queue, avoids conflicting ticks.

uint64_t ms_to_ticks(uint64_t ms) {
    return (ms + NUM_TICKS - 1) / NUM_TICKS; // TODO: Fix
}

int sleep_p(process_t *process, unsigned int ms) {
    uint8_t core_id = get_core_id();   

    // Ticks to sleep
    process->tick_delta = ms_to_ticks(ms);
    if (process->tick_delta <= 0)
        return OK;
    
    // Find dlist position
    struct list_head *cursor;
    list_for_each(cursor, &sleep_queue[core_id]) {
        process_t *entry = list_entry(cursor, process_t, block_list);

        if (process->tick_delta <= entry->tick_delta)
            break;

        process->tick_delta -= entry->tick_delta;
    }

    // Add to dlist
    list_add_tail(&process->block_list, cursor);

    // Adjust next offset
    if (list_has_next(&process->block_list, &sleep_queue[core_id])) {
        process_t *entry = list_entry(process->block_list.next, process_t, block_list);
        entry->tick_delta -= process->tick_delta;
    }

    return BLOCK;
}

void tick() {
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