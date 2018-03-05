#include "interrupts.h"

interrupt_vector_t vector_table_esr[4][ESR_ELx_EC_MAX + 1] = {
    [0 ... 3][0 ... ESR_ELx_EC_MAX]  = { .identify = NULL, .handle = undefined_handler }
    // TODO: Fill in with default stubs
};

interrupt_vector_t vector_table_int[4][INT_MAX + 1] = {
    [0 ... 3][0 ... INT_MAX] = { .identify = NULL, .handle = undefined_handler }
};

uint32_t *core_interrupt_src_irq = (uint32_t *) (VC_MMU_BASE | 0x60);
uint32_t *core_interrupt_src_fiq = (uint32_t *) (VC_MMU_BASE | 0x70);

void register_interrupt_handler(uint8_t core_id, bool sync, unsigned int entry, interrupt_vector_t vec) {
    (sync ? vector_table_esr[core_id] :
            vector_table_int[core_id])[entry] = vec;
}

void interrupt_handler_sync() {
    uint64_t esr;
    asm volatile("MRS %0, ESR_EL1" : "=r" (esr));

    interrupt_vector_t vec = vector_table_esr[get_core_id()][ESR_ELx_EC(esr)];
    if (vec.identify)  
        vec.identify();

    vec.handle();
}

void interrupt_handler_fiq() {
    uint8_t core_id = get_core_id();
    unsigned int src = core_interrupt_src_fiq[core_id];

    if (src == 0)
        undefined_handler();

    interrupt_vector_t vec = vector_table_int[core_id][__builtin_ffs(src) - 1];

    if (vec.identify)  
        vec.identify();

    vec.handle();
}

void interrupt_handler_irq() {
    uint8_t core_id = get_core_id();
    unsigned int src = core_interrupt_src_irq[core_id];

    if (src == 0)
        undefined_handler();
    
    interrupt_vector_t vec = vector_table_int[core_id][__builtin_ffs(src) - 1];

    if (vec.identify)  
        vec.identify();

    vec.handle();
}

void undefined_handler() {
    bool catch = true;
    while(catch); // Undefined Handler
}