#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "gpio.h"
#include "timer.h"
#include "peripheral.h"

// __attribute__ ((interrupt ("UDEF"))) void interrupt_udef() {
//     printf("UDEF\n");
// }

// typedef struct {
//     void (*handler)();
// } interrupt_descriptor_table_t, idt_t;

// extern idt_t idt[256];

static bool next_swi_state = true;

void __attribute__ ((interrupt ("SWI"))) interrupt_swi() {
    // volatile unsigned int i_code; // TODO: FIX

    // asm("ldr r0, [lr, #-4]");
    // asm("bic r0, #0xFF000000");
    // asm("mov %0, r0" : "=r"(i_code) : );

    // printf("SWI %x %d\n", i_code, i_code);
    // idt[i_code].handler();
    gpio_write(13, next_swi_state);
    next_swi_state = !next_swi_state;
}

// __attribute__ ((interrupt ("PABT"))) void interrupt_pabt() {
//     printf("PABT\n");
// }

// __attribute__ ((interrupt ("FIQ"))) void interrupt_fiq() {
//     printf("FIQ\n");
// }

static bool next_irq_state = true;

#define LT_IRQ			31
#define LT_ENABLE_IRQ	(1 << 29)
#define LT_ENABLE		(1 << 28)

/* Local Timer Reset and clear register */

#define LT_RESET_IRQ	(1 << 31)
#define LT_RELOAD		(1 << 30)

static bool last_state = false;

void interrupt_irq() {
    // __disable_interrupts();

    uint32_t irq_src = mmio_read(0x40000060);
    // gpio_write(13, false);
    // gpio_write(21, false);

    if (irq_src == (1 << 11)) {
        // timer_reset(0x038FFFF);
    // local_timer->control_status &= ~(1 << 29);
    // local_timer->control_status &= ~(1 << 28);
    // local_timer->irq_clear_reload = 1 << 31;

    // local_timer->control_status |= 1 << 28;  
    // local_timer->control_status |= 1 << 29; 
    // local_timer->control_status |= 0x038FFFF; 

        if (local_timer->control_status & (1 << 31)) {

            // if ((local_timer->control_status & (1 << 31)) == 0) {
            //     gpio_write(21, true);
            // }
            local_timer->irq_clear_reload = 1 << 31;
            gpio_write(13, last_state);
            last_state = !last_state;

            return;
        }
    } else {
        gpio_write(13, true);
        gpio_write(21, true);
        return;
    }
    // is timer int
    // if (irq_src != 0) {
    // // if (irq_src & ~(1 << 11)) {
    //     // local timer flag set?
    //     // if (local_timer->control_status & ~(1 << 31)) {
    //         timer_reset(0x038FFFF);
    //         gpio_write(13, next_irq_state);
    //         next_irq_state = !next_irq_state;
    //         return;
    //     // }
    //     //}
    // }

    // An error has happened
    // gpio_write(21, true);
    // printf("IRQ\n");
    
}

void interrupt_irq_other() {
    gpio_write(13, true);
    gpio_write(21, true);
}

// __attribute__ ((interrupt ("DABT"))) void interrupt_dabt() {
//     printf("DABT\n");
// }
