#include "include/gpio.h"
#include "include/peripheral.h"

gpio_t *gpio = (gpio_t *) GPIO_BASE;

bool valid_pin(unsigned int pin) {
    return pin >= 0 && pin <= 53;
}

extern int gpio_fsel(unsigned int pin, gpio_sel_t sel) {
    if (!valid_pin(pin)) return -1;

    gpio->sel[pin / 10] &= ~(0b111 << (3 * (pin % 10)));
    gpio->sel[pin / 10] |= (sel << (3 * (pin % 10)));

    return 0;
}

extern int gpio_read(unsigned int pin) {
    if (!valid_pin(pin)) return -1;    

    return (gpio->lev[pin / 32] & (1 << (pin % 32))) ? 1 : 0;
}

extern int gpio_write(unsigned int pin, bool val) {
    if (!valid_pin(pin)) return -1;    

    if (val)
        gpio->set[pin / 32] = (1 << (pin % 32));
    else
        gpio->clr[pin / 32] = (1 << (pin % 32));     

    return 0;
}

extern int gpio_pull(unsigned int pin, bool up, bool off) {
    if (!valid_pin(pin)) return -1;

    if (off)
        gpio->pud = 0x0;
    else 
        gpio->pud = up ? 0x2 : 0x1;
    
    // TODO: Less cycles, wait 150
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    gpio->pud_clk[pin / 32] = (1 << (pin % 32));
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    gpio->pud_clk[pin / 32] = 0;

    return 0;
}