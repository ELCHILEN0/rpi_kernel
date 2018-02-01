#include <stdint.h>

enum {
    L1_FAULT =                       0b00,
    L1_POINTER_L2 =                  0b01,
    L1_SECTION =        (0 << 18) | (0b10),
    L1_SUPERSECTION =   (1 << 18) | (0b10),

    L2_FAULT = 0b00,
    L2_LARGE_PAGE = 0b01,
    L2_SMALL_PAGE = 0b10,
} translation_table_entry_t;

enum {
    L1_PRW_URW = (0 << 15) | (0b11 << 10),
} access_permission_t;

enum {
    L1_STRONGLY_ORDERED =       (0b000 << 12) | (0 << 3) | (0 << 2),
    L1_DEVICE_SHARE =           (0b000 << 12) | (0 << 3) | (1 << 2),
    L1_NORMAL_WRITE_THROUGH_NO_ALLOC =   (0b000 << 12) | (1 << 3) | (0 << 2),
    L1_NORMAL_WRITE_BACK_NO_ALLOC =      (0b001 << 12) | (1 << 3) | (1 << 2),
    L1_NORMAL_NO_CACHE =        (0b001 << 12) | (0 << 3) | (0 << 2),
    L1_DEVICE_NO_SHARE =        (0b010 << 12) | (0 << 3) | (0 << 2),

    // From arm_cache.s (https://www.dropbox.com/s/tq6y2yod3p26yui/arm_cache.s)
    L1_DEVICE_010_00 = 0x00002c02,  // non-shareable device memory

    L1_NORMAL_000_10 = 0x00000c0a,	// outer and inner write-through, no write-allocate
    L1_NORMAL_000_11 = 0x00000c0e,	// outer and inner write-back, no write-allocate
    L1_NORMAL_001_11 = 0x00001c0e,	// outer and inner write-back, write-allocate
    L1_NORMAL_101_01 = 0x00005c06,	// outer and inner write-back, write-allocate
    L1_NORMAL_110_10 = 0x00006c0a, // outer and inner write-through, no write-allocate
    L1_NORMAL_111_11 = 0x00007c0e, // outer and inner write-back, no write-allocate
    L1_NORMAL_111_01 = 0x00007c06, // outer and inner write-back, outer no write-allocate, inner write allocate
    L1_NORMAL_101_11 = 0x00005c0e, // outer and inner write-back, outer write-allocate, inner no write-allocate
    L1_NORMAL_110_11 = 0x00006c0e, // outer write-through, inner write-back, no write-allocate
    L1_NORMAL_111_10 = 0x00007c0a, // outer and inner write-back, no write-allocate
} memory_type_t;

extern void init_linear_addr_map();
extern void enable_mmu(void);

// extern uint32_t __attribute__((aligned(0x4000))) l1_page_table[4096];
