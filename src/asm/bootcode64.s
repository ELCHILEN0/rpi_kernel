.section .text

// http://infocenter.arm.com/help/topic/com.arm.doc.dai0527a/DAI0527A_baremetal_boot_code_for_ARMv8_A_processors.pdf
.global _init_core
_init_core:
    // Drop to el1h
    bl enter_el1

    MOV    X0,  XZR
    MOV    X1,  XZR
    MOV    X2,  XZR
    MOV    X3,  XZR
    MOV    X4,  XZR
    MOV    X5,  XZR
    MOV    X6,  XZR
    MOV    X7,  XZR
    MOV    X8,  XZR
    MOV    X9,  XZR
    MOV    X10, XZR
    MOV    X11, XZR
    MOV    X12, XZR
    MOV    X13, XZR
    MOV    X14, XZR
    MOV    X15, XZR
    MOV    X16, XZR
    MOV    X17, XZR
    MOV    X18, XZR
    MOV    X19, XZR
    MOV    X20, XZR
    MOV    X21, XZR
    MOV    X22, XZR
    MOV    X23, XZR
    MOV    X24, XZR
    MOV    X25, XZR
    MOV    X26, XZR
    MOV    X27, XZR
    MOV    X28, XZR
    MOV    X29, XZR
    MOV    X30, XZR

    // Disable trapping of accessing in EL3 and EL2.
    // MSR    CPTR_EL3, XZR
    // MSR    CPTR_EL3, XZR

    // Disable access trapping in EL1 and EL0.
    MOV X1, #(0x3 << 20)
    MSR CPACR_EL1, X1
    ISB

    // Initialize exception table
    LDR X1, =vector_table_el1
    MSR VBAR_EL1, X1

    // Jump to the core init vector
    BL get_core_id

    ADR X1, _core_vectors
    MOV X2, #4
    MADD X1, X0, X2, X1
    BR X1

hang:
    b hang

_core_vectors:
    B _init_core_0
    B _init_core_1
    B _init_core_2
    B _init_core_3

_init_core_0:
    MSR SPSel, #1
    ldr x0, =__el1_stack_end_core_0
    MOV sp, x0
    
    // Branch to higher level c routines
    bl cstartup
    b hang

_init_core_1:
    MSR SPSel, #1
    ldr x0, =__el1_stack_end_core_1
    mov sp, x0

    b cinit_core

_init_core_2:
    MSR SPSel, #1
    ldr x0, =__el1_stack_end_core_2
    mov sp, x0
    
    b cinit_core

_init_core_3:
    MSR SPSel, #1
    ldr x0, =__el1_stack_end_core_3
    mov sp, x0
    
    b cinit_core

enter_el1:
    // aarch64
    MRS x0, HCR_EL2
    ORR x0, x0, #(1<<31)
	MSR HCR_EL2, x0

    // reset value
	MSR	SCTLR_el1, XZR

    // MODE = EL1H, DAIF = 0
	MOV x0, #0b00101 
    MSR SPSR_EL2, x0 

    // return addr
	MSR	ELR_el2, x30
	ERET

.global enter_el0
enter_el0:
/*
    mov x0, sp
    msr sp_el0, x0 // SP_EL0 = SP_EL1
*/
    MOV x0, #0b00000 
    MSR SPSR_EL1, x0 

    MSR	ELR_el1, x30
	ERET

.global get_core_id
get_core_id:
    MRS X0, MPIDR_EL1
    UBFX X0, X0, #0, #2
    RET

.global __enable_interrupts
__enable_interrupts:
    MSR DAIFclr, #(0x1 | 0x2 | 0x4)
    RET

.global __disable_interrupts
__disable_interrupts:
    MSR DAIFset, #(0x1 | 0x2 | 0x4)
    RET

// Put a 64-bit value with little endianness.
.macro PUT_64B high, low
    .word \low
    .word \high
.endm

.macro TABLE_ENTRY PA, ATTR
    PUT_64B   \ATTR, (\PA) + 0x3
.endm

.macro BLOCK_1GB PA, ATTR_HI, ATTR_LO
    PUT_64B \ATTR_HI, ((\PA) & 0xC0000000) | \ATTR_LO | 0x1
.endm

.macro BLOCK_2MB PA, ATTR_HI, ATTR_LO
    PUT_64B \ATTR_HI, ((\PA) & 0xFFE00000) | \ATTR_LO | 0x1 
.endm

.align 12
ttb0_base:
TABLE_ENTRY level2_pagetable, 0
BLOCK_1GB 0x40000000, 0, 0x740
BLOCK_1GB 0x80000000, 0, 0x740
BLOCK_1GB 0xC0000000, 0, 0x740

.align 12
level2_pagetable:
.set ADDR, 0x000
.rept 0x1F8
BLOCK_2MB (ADDR << 20), 0, 0x708 // Normal Memory
.set ADDR, ADDR+2
.endr
.rept 0x8
BLOCK_2MB (ADDR << 20), 0, 0x740 // Device Memory
.set ADDR, ADDR+2
.endr

.global enable_mmu
enable_mmu:
    LDR X1, = 0x3520
    MSR TCR_EL1, X1

    /* 
    Memory attributes
        0 => 0x00 = device nGnRnE
        1 => 0x44 = normal outer and inner non-cacheable
        2 => 0x5D = normal outer and inner write-back, write-allocate
    */
    LDR X1, =0x5D4400
    MSR MAIR_EL1, X1

    ADR X0, ttb0_base
    MSR TTBR0_EL1, X0

    // NOTE: CPU Extended Control Register (S3_1_C15_C2_1) SMPEN already set

    MRS X0, SCTLR_EL1
    ORR X0, X0, #(0x1 << 2)
    ORR X0, X0, #(0x1 << 12)
    ORR X0, X0, 0x1
    MSR SCTLR_EL1, X0
    DSB SY
    ISB
    RET
