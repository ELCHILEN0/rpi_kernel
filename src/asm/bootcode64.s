.section .text

// http://infocenter.arm.com/help/topic/com.arm.doc.dai0527a/DAI0527A_baremetal_boot_code_for_ARMv8_A_processors.pdf
.global _init_core
_init_core:
    // Drop to el1
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

    // Jump to the core init vector
    MRS x0, MPIDR_EL1
    UBFX x0, x0, #0, #2

    ADR x1, _core_vectors
    MOV x2, #4
    MADD x1, x0, x2, x1
    BR x1

hang:
    b hang

_core_vectors:
    B _init_core_0
    B _init_core_1
    B _init_core_2
    B _init_core_3

_init_core_0:
#if 0
    //ldr x0, =_vectors_el1
    //msr vbar_el1, x0
#endif
    ldr x0, =__el1_stack_end_core_0
    mov sp, x0

/*
    bl enter_el0

    ldr x0, =__el0_stack_end_core_0
    mov sp, x0
*/

    LDR    X1, =vector_table_el1
    MSR    VBAR_EL1, X1

    /**
    * Finally branch to higher level c routines.
    */
    bl cstartup
    b hang

_init_core_1:
    ldr x0, =__el1_stack_end_core_1
    mov sp, x0

    b cinit_core

_init_core_2:
    ldr x0, =__el1_stack_end_core_2
    mov sp, x0
    
    b cinit_core

_init_core_3:
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

.global __enable_interrupts
__enable_interrupts:
    MSR DAIFset, #(0x1 | 0x2 | 0x4)
    RET

.global __disable_interrupts
__disable_interrupts:
    MSR DAIFclr, #(0x1 | 0x2 | 0x4)
    RET
