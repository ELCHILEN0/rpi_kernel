.section .text

// See ARM section A2.2 (Processor Modes)
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVC,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F

// See ARM section A2.5 (Program status registers)
.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

.equ	SCTLR_ENABLE_DATA_CACHE,        0x4
.equ	SCTLR_ENABLE_BRANCH_PREDICTION, 0x800
.equ	SCTLR_ENABLE_INSTRUCTION_CACHE, 0x1000

.global _vectors
_vectors:
    b _init_core
    nop
    nop
    nop
    nop
    nop
    nop
    nop

hang:
    b hang

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

.global _init_core
_init_core:

/*
    // R0 = System Control Register
    mrc p15,0,r0,c1,c0,0
	
    // Enable caches and branch prediction
    orr r0,#SCTLR_ENABLE_BRANCH_PREDICTION
    orr r0,#SCTLR_ENABLE_DATA_CACHE
    orr r0,#SCTLR_ENABLE_INSTRUCTION_CACHE

    // System Control Register = R0
    mcr p15,0,r0,c1,c0,0
  */
    b _init_core_0
    b hang

_core_vectors:
    .word _init_core_0
    .word _init_core_1
    .word _init_core_2
    .word _init_core_3

_init_core_0:

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
