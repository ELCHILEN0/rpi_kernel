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

    // Jump to core init
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

    /**
    * Finally branch to higher level c routines.
    */
    bl cstartup
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

enable_interrupts:
    MSR DAIFset, #(0x1 | 0x2 | 0x4)

disable_interrupts:
    MSR DAIFclr, #(0x1 | 0x2 | 0x4)
