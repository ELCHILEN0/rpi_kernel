#include "cache.h"
#include "peripheral.h"

#include <stdbool.h>

static volatile __attribute__ ((aligned (0x4000))) uint32_t l1_page_table[4096];

void init_linear_addr_map() {
    // uint32_t pbase;
    // asm("MRC p15, 1, %0, c15, c3, 0" :: "r" (pbase));
    // printf("PBASE = 0x%X\r\n");

    // @ 31                 20 19  18  17  16 15  14   12 11 10  9  8     5   4    3 2   1 0
    // @ |section base address| 0  0  |nG| S |AP2|  TEX  |  AP | P | Domain | XN | C B | 1 0|

    uint32_t base;
    for (base = 0; base < 1024 - 16; base++) {
        // l1_page_table[base] = base << 20 | L1_NORMAL_001_11 | L1_PRW_URW | L1_SECTION;
        // TODO: outer and inner write-back, write-allocate
        l1_page_table[base] = base << 20 | L1_PRW_URW | L1_SECTION;
        l1_page_table[base] |= (1 << 12) | (1 << 2);    // TEX[0] C B = 101
        l1_page_table[base] |= (1 << 16);               // S = 1
    }

    for (; base < 1025; base++) {
        // l1_page_table[base] = base << 20 | L1_DEVICE_010_00 | L1_PRW_URW | L1_SECTION;
        l1_page_table[base] = base << 20 | L1_PRW_URW | L1_SECTION;
        // l1_page_table[base] |= (1 << 12) | (1 << 2);    // TEX[0] C B = 101
        l1_page_table[base] |= (1 << 16);               // S = 1
        l1_page_table[base] |= (1 << 4);                // XN = 1
    }

    // for (int base = 0; base < 4096; base++) {
    //     // l1_page_table[base] = ((0xFFF00000) & (base << 20)) | L1_NORMAL_001_11 | L1_PRW_URW | L1_SECTION;
    //     if ((base << 20) < PERIPHERAL_BASE) {
    //         l1_page_table[base] = ((0xFFF00000) & (base << 20)) | L1_NORMAL_001_11 | L1_PRW_URW | L1_SECTION ;
    //     } else {
    //         l1_page_table[base] = ((0xFFF00000) & (base << 20)) | (1 << 16) | (0b01 << 2) | L1_PRW_URW | L1_SECTION;
    //     }
    // }

    for (; base < 4096; base++) {
        l1_page_table[base] = 0;
    }
}

/*

@
@ Bits[31:20]   - Top 12 bits of VA is pointer into table
@ nG[17]=0      - Non global, enables matching against ASID in the TLB when set.
@ S[16]=0       - Indicates normal memory is shared when set.
@ AP2[15]=0
@ AP[11:10]=11  - Configure for full read/write access in all modes
@ TEX[14:12]=
@ CB[3:2]=
@
@ IMPP[9]=0     - Ignored
@ Domain[5:8]=0 - Set all pages to use domain 0
@ XN[4]=0       - Execute never disabled
@ Bits[1:0]=10  - Indicate entry is a 1MB section


And an important thing, be careful with identity mapping when you reach MMIO_BASE.
You'll have to use a different AttrIndex and shareability.
For normal memory it should be inner shareable and AttrIndex should point to a 
MAIR value of 0xCC or 0xFF (depending on you want alloc or not), while for 
device memory it must be outer shareable and AttrIndex must point to a 
MAIR value of 0x04 in order to work properly.


MAIR...

*/

void enable_mmu(void)
{    
    uint32_t prrr;
    asm volatile("MRC p15, 0, %0, c10, c2, 0" : "=r" (prrr));
    // printf("PRRR=0x%X\r\n", prrr);
    asm volatile("MCR p15, 0, %0, c10, c2, 0" :: "r" (prrr));

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
    // asm("ISB");

    unsigned auxctrl;
    asm volatile ("mrc p15, 0, %0, c1, c0,  1" : "=r" (auxctrl));
    // printf("ax = 0x%X\r\n", auxctrl);
    auxctrl |= (1 << 6); // Enable SMP
    asm volatile ("mcr p15, 0, %0, c1, c0,  1" :: "r" (auxctrl));

    asm volatile ("mcr p15, 0, %0, c2, c0, 2" :: "r" (0)); // TTBR0
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/CHDGIJFB.html
    asm volatile ("mcr p15, 0, %0, c2, c0, 0" :: "r" ((uint32_t) l1_page_table) : "memory");
    asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (~0)); // DACR (allow everyone)
    asm volatile("isb");

    asm volatile("MRC p15, 0, %0, c1, C0, 0" : "=r" (control));
    // printf("CTRL: 0x%X\r\n", control);
    control |= (1 << 0);    // Set M to enable MMU
    control |= (1 << 2);    // Set C to enable D Cache
    // control |= (1 << 12);   // Set I to enable I Cache
    control |= (1 << 11);   // Set Z to enable branch prediction
    control |= (1 << 28);   // TEX REMAP TEX[0] + C + B
    asm volatile("MCR p15, 0, %0, C1, C0, 0" :: "r" (control));
    asm volatile("isb");
}