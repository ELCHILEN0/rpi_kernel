#include <stdint.h>

typedef volatile struct {
    uint32_t sel[6];
    uint32_t _RESERVED_0;
    uint32_t set[2];
    uint32_t _RESERVED_1;
    uint32_t clr[2];
    uint32_t _RESERVED_2;
    uint32_t lev[2];
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

extern gpio_t *gpio;