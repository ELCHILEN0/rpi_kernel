#ifndef INTERRUPTS_H
#define INTERRUPTS_H

typedef struct {
    void (*handler)(void);
} interrupt_vector_t;

extern void init_vector_tables();
extern void register_interrupt_handler(interrupt_vector_t vector_table[], unsigned int i, void (*handler)());

extern interrupt_vector_t vector_table_svc[];
extern interrupt_vector_t vector_table_irq[];
#endif