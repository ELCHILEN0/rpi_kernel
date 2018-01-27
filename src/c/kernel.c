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

uint32_t act_message[] = {32, 0, 0x00038041, 8, 0, 130, 0, 0};

extern void __enable_interrupts(void);
extern void __disable_interrupts(void);
extern void _init_core(void);

static spinlock_t slave_lock;

void slave_core() {
    enable_mmu();

    int core_id = get_core_id();

    __spin_lock(&slave_lock);
    printf("[core] Code running on core %d\r\n", core_id);
    __spin_unlock(&slave_lock);

    switch (core_id) {
        case 1:
            gpio_write(13, true);
            break;

        case 2:
            gpio_write(19, true);
            break;

        case 3:
            gpio_write(21, true);
            break;
    }

    while (true);
}

void context_switch() {
    static bool next_blinker_state = true;
    gpio_write(21, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void time_slice() {
    local_timer_reset();

    static bool next_blinker_state = true;
    gpio_write(13, next_blinker_state);
    next_blinker_state = !next_blinker_state;
}

void init_jtag() {
    // JTAG
    // gpio_fsel(22, SEL_ALT4);
    // gpio_fsel(24, SEL_ALT4);
    // gpio_fsel(25, SEL_ALT4);
    // gpio_fsel(27, SEL_ALT4);
    // gpio_fsel(4, SEL_ALT5);

    // gpio_fsel(22, SEL_ALT4); // TRST
    // gpio_fsel(23, SEL_ALT4); // RTCK
    // gpio_fsel(24, SEL_ALT4); // TDO
    // gpio_fsel(25, SEL_ALT4); // TCK
    // gpio_fsel(26, SEL_ALT4); // TDI
    // gpio_fsel(27, SEL_ALT4); // TMS
}

void kernel_main ( uint32_t r0, uint32_t r1, uint32_t atags ) {
	(void) r0;
	(void) r1;
	(void) atags;

    // OK status (use till GPIO working)
    act_message[6] = 1;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);

    gpio_fsel(5, SEL_INPUT);
    gpio_fsel(6, SEL_OUTPUT);
    gpio_fsel(13, SEL_OUTPUT);
    gpio_fsel(19, SEL_OUTPUT);
    gpio_fsel(21, SEL_OUTPUT);

    uart_init(9600);

    printf("[kernel] Kernel started on core %d\r\n", get_core_id());
    init_linear_addr_map();
    enable_mmu();
    printf("[kernel] MMU enabled\r\n");
    // __enable_interrupts();

    slave_lock.flag = 0;
    
    core_enable(1, (uint32_t) _init_core);
    core_enable(2, (uint32_t) _init_core);
    core_enable(3, (uint32_t) _init_core);

    register_interrupt_handler(vector_table_svc, 0x80, &context_switch);
    register_interrupt_handler(vector_table_svc, 0x81, context_switch);
    register_interrupt_handler(vector_table_irq, 11, time_slice);

    local_timer_interrupt_routing(0);
    local_timer_start(0x038FFFF);
 
    // printf("[kernel] Jumping to interrupt 0x80.\r\n");
    // asm("SVC 0x80");
    // printf("[kernel] Returned from interrupt.\r\n");

    bool output_state = true;

    while(true) {
        gpio_write(6, output_state);
        output_state = !output_state;

        bool input_state = gpio_read(5);
        while (input_state == gpio_read(5)); // Poll till pressed
        while (input_state != gpio_read(5)); // Poll till unpressed
    }

    // Error status
    act_message[6] = 0;
    mailbox_write(mailbox0, MB0_PROPERTY_TAGS_ARM_TO_VC, (uint32_t) &act_message);
}