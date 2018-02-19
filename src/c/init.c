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

#include "kernel/kernel.h"

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);
extern void _init_core(void);

spinlock_t print_lock;

void test_handler() {
    static bool next_blinker_state = true;    
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;    

    uart_putc('.');
    core_timer_rearm(19200000);
}

void interrupt_handler() {
    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;

    // _int_syscall();
}

void time_slice() {

    static bool next_blinker_state = true;
    gpio_write(13, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void master_core () {
    __spin_lock(&print_lock);
    printf("[core%d] Executing from 0x%X\r\n", get_core_id(), master_core);
    __spin_unlock(&print_lock);

    // register_interrupt_handler(vector_table_svc, 0x80, _int_syscall);
    // register_interrupt_handler(vector_table_svc, 0x81, interrupt_handler);
    // register_interrupt_handler(vector_table_irq, 11, time_slice);

    __enable_interrupts();

    // kernel_init();

    while (true) {
        for (int i = 0; i < 0x100000 * 30; i++);
        gpio_write(5, true);

        for (int i = 0; i < 0x100000 * 30; i++);
        gpio_write(5, false);
    }
}

void slave_core() {
    int core_id = get_core_id();
    int core_gpio[3] = { 6, 13, 19 };

    __spin_lock(&print_lock);
    printf("[core%d] Executing from 0x%X!\r\n", core_id, slave_core);
    __spin_unlock(&print_lock);

    while (true) {
        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        gpio_write(core_gpio[core_id - 1], true);

        for (int i = 0; i < 0x10000 * (core_id + 1) * 30; i++);
        gpio_write(core_gpio[core_id - 1], false);  
    }
}

extern void enter_el0();
extern void enable_mmu();

void cinit_core(void) {    
    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    gpio_fsel(21, SEL_OUTPUT);
    
    __enable_interrupts();       
    core_timer_init( CT_CTRL_SRC_APB, CT_CTRL_INC2, 0x80000000);
    // core_timer_init( CT_CTRL_INC1, CT_CTRL_SRC_CRS, 0x00FFFFFF );
    core_timer_interrupt_routing(0, CT_IRQ_NON_SECURE);
    core_timer_rearm(19200000);

    while(true);
    // while (true) {
    //     uint32_t code = mmio_read(0x40000060);
    //     if (code == 0) continue;
    //     test_handler();
    //     // local_timer_start(0x038FFFF);
    // }

    int core_id = get_core_id();
    switch(core_id) {
        case 0:
        {
            core_enable(1, (uint64_t) _init_core);
            core_enable(2, (uint64_t) _init_core);
            core_enable(3, (uint64_t) _init_core);   

            enable_mmu();                

            init_vector_tables();

            register_interrupt_handler(vector_table_irq, 0x80, test_handler);

            gpio_fsel(5, SEL_OUTPUT);
            gpio_fsel(6, SEL_OUTPUT);
            gpio_fsel(13, SEL_OUTPUT);
            gpio_fsel(19, SEL_OUTPUT);
            gpio_fsel(21, SEL_OUTPUT);

            gpio_write(5, true);
            gpio_write(6, true);
            gpio_write(13, true);
            gpio_write(19, true);
            gpio_write(21, true);

            uart_init(115200); // TODO: Config flag may enable this

            master_core();
        }
        break;

        default:
            enable_mmu();
            slave_core();
            break;
    }
    
    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}