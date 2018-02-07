.section .text


/**
 * On startup the armstub runs which is located from 0x0 - 0x100
 * TODO (Optional): Rewite armstub to follow https://www.kernel.org/doc/Documentation/arm64/booting.txt semantics
 *
 * armstubs: https://github.com/raspberrypi/tools/tree/master/armstubs
 * - HYP
 * - Core 0 jumps to 0x8000, 0x80000 on aarch64
 * - Cores 1-3 are stuck in a WFE loop
 * - On SVR they jump to the address in mailbox 3

 * Therefore common startup code should:
 * - HYP -> SVR (no need for virtualization extensions)
 * - Setup execution level stacks (this varies on aarch64)
 * - Jump to execution code

 * Initialization routine is as follows:
 * - HYP -> SVC (all cores)
 * - Jump to core vector to perform core specific initialization
 * - Jump to core execution
 */

// http://infocenter.arm.com/help/topic/com.arm.doc.dai0527a/DAI0527A_baremetal_boot_code_for_ARMv8_A_processors.pdf
.global _init_core
_init_core:
    b _init_core_0

hang:
    b hang

_core_vectors:
    .word _init_core_0
    .word _init_core_1
    .word _init_core_2
    .word _init_core_3

_init_core_0:
    //bl cinit_core

#if 0
    //ldr x0, =_vectors_el1
    //msr vbar_el1, x0
#endif

#if 1
    bl enter_el1
    nop
    nop
#if 1
    bl enter_el0
#endif
#endif

    /**
    * Finally branch to higher level c routines.
    */
    bl cinit_core
    b hang

_init_core_1:
  b hang

_init_core_2:
  b hang

_init_core_3:
  b hang

enter_el1:
    // aarch64
    MRS x0, HCR_EL2
    ORR x0, x0, #(1<<31)
	MSR HCR_EL2, x0

    // reset value
	msr	SCTLR_el1, XZR

    // MODE = EL1H, DAIF = 0
	MOV x0, #0b00101 
    MSR SPSR_EL2, x0 

    // return addr
	MSR	ELR_el2, x30
	ERET


enter_el0:
/*
    mov x0, sp
    msr sp_el0, x0 // SP_EL0 = SP_EL1
*/
    MOV x0, #0b00000 
    MSR SPSR_EL1, x0 

    MSR	ELR_el1, x30
	ERET

/*
enable_interrupts:
    msr daifset #(0x1 | 0x2 | 0x4)

disable_interrupts:
    msr daifsclr #(0x1 | 0x2 | 0x4)
*/