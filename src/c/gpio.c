#include "gpio.h"
#include "peripheral.h"

extern gpio_t *gpio = (gpio_t *) GPIO_BASE;

// TODO: Valid pin ranges...

extern void gpio_fsel(unsigned int pin, enum gpio_fsel_t sel) {
    gpio->sel[pin / 10] &= ~(0b111 << (3 * (pin % 10)));
    gpio->sel[pin / 10] |= (sel << (3 * (pin % 10)));
}

extern bool gpio_read(unsigned int pin) {
    return (gpio->lev[pin / 32] & (1 << (pin % 32))) ? true : false;
}

extern void gpio_write(unsigned int pin, bool val) {
    if (val)
        gpio->clr[pin / 32] |= (1 << (pin % 32));
    else
        gpio->set[pin / 32] |= (1 << (pin % 32));     
}