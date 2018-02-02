#include <stdint.h>
#include <stdbool.h>

typedef volatile struct {
    uint32_t sel[6];
    uint32_t _RESERVED_0;
    uint32_t set[2];
    uint32_t _RESERVED_1;
    uint32_t clr[2];
    uint32_t _RESERVED_2;
    const uint32_t lev[2];
    uint32_t _RESERVED_3;
    uint32_t eds[2];
    uint32_t _RESERVED_4;
    uint32_t ren[2];
    uint32_t _RESERVED_5;
    uint32_t fen[2];
    uint32_t _RESERVED_6;
    uint32_t hen[2];
    uint32_t _RESERVED_7;
    uint32_t len[2];
    uint32_t _RESERVED_8;
    uint32_t aren[2];
    uint32_t _RESERVED_9;
    uint32_t afen[2];
    uint32_t _RESERVED_10;
    uint32_t pud;
    uint32_t pud_clk[2];
    uint32_t _RESERVED_11;
    uint8_t test; // TODO: 4 instead of 8
} gpio_t;

typedef enum {
    SEL_INPUT = 0b000,
    SEL_OUTPUT = 0b001,
    SEL_ALT0 = 0b100,
    SEL_ALT1 = 0b101,
    SEL_ALT2 = 0b110,
    SEL_ALT3 = 0b111,
    SEL_ALT4 = 0b011,
    SEL_ALT5 = 0b010
} gpio_sel_t;

extern gpio_t *gpio;

/**
 * \brief Assign a pin function selector.
 * \param pin - a pin to set
 * \param sel - a gpio selector function
 * \return 0 on success
 * \return -1 on error
 */
extern int gpio_fsel(unsigned int pin, gpio_sel_t sel);

/**
 * \brief Read a pin status
 * \param pin - a pin to read
 * \return 0 on LOW
 * \return 1 on HIGH
 * \return -1 on error
 */
extern int gpio_read(unsigned int pin);

/**
 * \brief Write a pin status
 * \param pin - a pin to read
 * \param high - a new pin state
 * \return 0 on success
 * \return -1 on error
 */
extern int gpio_write(unsigned int pin, bool high);

/**
 * \brief Control the actuation of internal pull-up/down on the respective GPIO pins.
 * \param pin - a pin to set
 * \param up - pull up or down
 * \param off - if true override the previous param, disables pull up or down
 * \return 0 on success
 * \return -1 on error
 */
extern int gpio_pull(unsigned int pin, bool up, bool off);