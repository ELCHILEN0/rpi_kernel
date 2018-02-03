#ifndef CACHE_H
#define CACHE_H
#include <stdint.h>

enum {
    L1_FAULT =                      0x0,
    L1_POINTER_L2 =                 0x1,
    L1_SECTION =        (0x0 << 18) | 0x2,
    L1_SUPERSECTION =   (0x1 << 18) | 0x2,

    L2_FAULT = 0x0,
    L2_LARGE_PAGE = 0x1,
    L2_SMALL_PAGE = 0x2,
} translation_table_entry_t;

enum {
    L1_PRW_URW = (0x0 << 15) | (0x3 << 10),
} access_permission_t; 

/**
 * Memory region attributes defined in
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0464d/B4BEIDGHFH.html
 * other attributes defined in
 * https://www.dropbox.com/s/tq6y2yod3p26yui/arm_cache.s
 */
enum {
    L1_STRONG_000_00 = 0x0 << 12 | 0x0 << 2, // strongly ordered (shareable)
    L1_DEVICE_000_01 = 0x0 << 12 | 0x1 << 2, // shareable device
    L1_NORMAL_000_10 = 0x0 << 12 | 0x2 << 2, // outer and inner write-through, no write-allocate
    L1_NORMAL_000_11 = 0x0 << 12 | 0x3 << 2, // outer and inner write-through, no write-allocate
    L1_NORMAL_001_00 = 0x1 << 12 | 0x0 << 2, // outer and inner non-cacheable
    L1_NORMAL_001_10 = 0x1 << 12 | 0x2 << 2, // outer and inner write-back, write-allocate
    L1_DEVICE_010_00 = 0x2 << 12 | 0x0 << 2, // non-shareable device
} memory_type_t;

extern void init_linear_addr_map();
extern void enable_mmu(void);
extern void disable_mmu(void);

extern uint32_t __attribute__((aligned(0x4000))) l1_page_table[4096];
#endif