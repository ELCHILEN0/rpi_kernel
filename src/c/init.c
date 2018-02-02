#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "peripheral.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "interrupts.h"

#include "mailbox.h"
#include "multicore.h"
#include "cache.h"

#include "kernel/kernel.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);
extern void _init_core(void);

spinlock_t print_lock;

extern void _InterruptEntryPoint(void);

void interrupt_handler() {
    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;

    _InterruptEntryPoint();
}

void time_slice() {
    local_timer_reset();

    static bool next_blinker_state = true;
    gpio_write(13, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void init_jtag() {
    gpio_pull(22, false, true);
    gpio_pull(24, false, true);
    gpio_pull(25, false, true);
    gpio_pull(26, false, true);
    gpio_pull(27, false, true);

    gpio_fsel(22, SEL_ALT4); // TRST
    gpio_fsel(24, SEL_ALT4); // TDO
    gpio_fsel(25, SEL_ALT4); // TCK
    gpio_fsel(26, SEL_ALT4); // TDI
    gpio_fsel(27, SEL_ALT4); // TMS
}

void master_core () {
    __spin_lock(&print_lock);
    printf("[core%d] Executing from 0x%X\r\n", get_core_id(), master_core);
    __spin_unlock(&print_lock);

    register_interrupt_handler(vector_table_svc, 0x80, &interrupt_handler);
    register_interrupt_handler(vector_table_svc, 0x81, interrupt_handler);
    register_interrupt_handler(vector_table_irq, 11, time_slice);

    local_timer_interrupt_routing(0);
    local_timer_start(0x038FFFF);

    kernel_init();

    while (true) {
        for (int i = 0; i < 0x10000 * 30; i++);
        gpio_write(5, true);

        for (int i = 0; i < 0x10000 * 30; i++);
        gpio_write(5, false);
    }
}

void slave_core() {
    int core_id = get_core_id();
    int core_gpio[3] = { 6, 13, 19 };

    __spin_lock(&print_lock);
    printf("[core%d] Executing from 0x%X!\r\n", core_id, slave_core);
    for (int i = 0; i < 0x1000000; i++) { asm("nop"); }
    __spin_unlock(&print_lock);

    while (true) {
        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        gpio_write(core_gpio[core_id - 1], true);

        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        gpio_write(core_gpio[core_id - 1], false);
  
    }
}

void cinit_core( ) {
    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    int core_id = get_core_id();
    switch(core_id) {
        case 0:
        {
            init_vector_tables();

            gpio_fsel(5, SEL_OUTPUT);
            gpio_fsel(6, SEL_OUTPUT);
            gpio_fsel(13, SEL_OUTPUT);
            gpio_fsel(19, SEL_OUTPUT);
            gpio_fsel(21, SEL_INPUT);

            uart_init(115200);
            init_jtag();

            printf("[core%d] Started...\r\n", core_id, master_core);

            init_linear_addr_map();
            enable_mmu();       

            core_enable(1, (uint32_t) _init_core);
            core_enable(2, (uint32_t) _init_core);
            core_enable(3, (uint32_t) _init_core);     

            master_core(); // No more coprocessor changes, J-TAG entrypoint
        }
        break;

        default:
            enable_mmu();
            slave_core(); // No more coprocessor changes, J-TAG entrypoint
            break;
    }
    
    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}