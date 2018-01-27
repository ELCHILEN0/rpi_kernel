#include "cache.h"
#include "peripheral.h"

#include <stdio.h>

uint32_t l1_page_table[4096];

void enable_mmu(void)
{
    // TODO: Move to different func...
    for (int base = 0; base < 4096; base++) {
        if ((base << 20) < PERIPHERAL_BASE) {
            l1_page_table[base] = base << 20 | L1_NORMAL_001_11 | L1_PRW_URW | L1_SECTION ;
        } else {
            l1_page_table[base] = base << 20 | L1_DEVICE_SHARE | L1_PRW_URW | L1_SECTION;
        }
    }

    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344k/Bahfeedc.html
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
    asm("ISB");

    asm volatile ("mcr p15, 0, %0, c2, c0, 2" :: "r" (0)); // TTBR0
    asm volatile ("mcr p15, 0, %0, c2, c0, 0" :: "r" (l1_page_table) : "memory");
    asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (~0)); // DACR (allow everyone)

    asm volatile("MRC p15, 0, %0, c1, C0, 0" : "=r" (control));
    control |= (1 << 0);    // Set M to enable MMU
    control |= (1 << 2);    // Set C to enable D Cache
    control |= (1 << 12);   // Set I to enable I Cache
    control |= (1 << 11);   // Set Z to enable branch prediction
    asm volatile("MCR p15, 0, %0, C1, C0, 0" :: "r" (control));
}