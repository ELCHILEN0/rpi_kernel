#include "gpio.h"
#include "peripheral.h"

gpio_t *gpio = (gpio_t *) GPIO_BASE;

bool valid_pin(unsigned int pin) {
    return pin >= 0 && pin <= 53;
}

extern int gpio_fsel(unsigned int pin, gpio_sel_t sel) {
    if (!valid_pin) return -1;

    gpio->sel[pin / 10] &= ~(0b111 << (3 * (pin % 10)));
    gpio->sel[pin / 10] |= (sel << (3 * (pin % 10)));

    return 0;
}

extern int gpio_read(unsigned int pin) {
    if (!valid_pin) return -1;    

    return (gpio->lev[pin / 32] & (1 << (pin % 32))) ? 1 : 0;
}

extern int gpio_write(unsigned int pin, bool val) {
    if (!valid_pin) return -1;    

    if (val)
        gpio->set[pin / 32] = (1 << (pin % 32));
    else
        gpio->clr[pin / 32] = (1 << (pin % 32));     

    return 0;
}