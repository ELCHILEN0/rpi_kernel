#include "cache.h"
#include "peripheral.h"

#include <stdbool.h>

uint32_t l1_page_table[4096];

// typedef volatile struct {
//     uint8_t 
// } l1_section;

void init_linear_addr_map() {

    // @ 31                 20 19  18  17  16 15  14   12 11 10  9  8     5   4    3 2   1 0
    // @ |section base address| 0  0  |nG| S |AP2|  TEX  |  AP | P | Domain | XN | C B | 1 0|

    uint32_t base;
    for (base = 0; base < 1024 - 16; base++) {
        l1_page_table[base] = base << 20 | L1_PRW_URW | L1_SECTION;
        l1_page_table[base] |= L1_NORMAL_001_10;
        l1_page_table[base] |= (1 << 16);               
    }

    // TODO: Determine from system registers...
    for (; base < 1025; base++) {
        l1_page_table[base] = base << 20 | L1_PRW_URW | L1_SECTION;
        l1_page_table[base] |= L1_DEVICE_000_01; 
        l1_page_table[base] |= (1 << 16);               
        l1_page_table[base] |= (1 << 4);                
    }

    for (; base < 4096; base++) {
        l1_page_table[base] = 0;
    }
}

void enable_mmu(void)
{    
    uint32_t control;
    asm volatile("MRC p15, 0, %0, c1, c0, 0" : "=r" (control));
    control &= ~(1 << 0);   // Clear M to disable MMU
    control &= ~(1 << 2);   // Clear C to disable D Cache
    control &= ~(1 << 12);  // Clear I to disable I Cache
    control &= ~(1 << 11);  // Clear Z to disable branch prediction
    asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r" (control));
    
    // Flush L1 Cache, TLB, Branch Prediction
    asm("MCR p15, 0, %0, c7, c5, 0" :: "r" (0));
    asm("MCR p15, 0, %0, c7, c5, 6" :: "r" (0));
    asm("MCR p15, 0, %0, c8, c7, 0" :: "r" (0));

    unsigned auxctrl;
    asm volatile ("mrc p15, 0, %0, c1, c0,  1" : "=r" (auxctrl));
    auxctrl |= (1 << 6); // Enable SMP (Should be unused...)
    asm volatile ("mcr p15, 0, %0, c1, c0,  1" :: "r" (auxctrl));

    asm volatile ("mcr p15, 0, %0, c2, c0, 2" :: "r" (0)); // TTBR0
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/CHDGIJFB.html
    asm volatile ("mcr p15, 0, %0, c2, c0, 0" :: "r" ((uint32_t) l1_page_table) : "memory");
    asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (~0)); // DACR (allow everyone)
    asm volatile("isb");

    asm volatile("MRC p15, 0, %0, c1, C0, 0" : "=r" (control));
    control |= (1 << 0);    // Set M to enable MMU
    control |= (1 << 2);    // Set C to enable D Cache
    control |= (1 << 12);   // Set I to enable I Cache
    control |= (1 << 11);   // Set Z to enable branch prediction
    // control |= (1 << 28);   // TEX_REMAP = TEX[0] + C + B instead of TEX[2:0] + C + B
    asm volatile("MCR p15, 0, %0, C1, C0, 0" :: "r" (control));
    asm volatile("isb");
}