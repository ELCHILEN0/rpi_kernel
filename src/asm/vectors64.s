// https://developer.arm.com/products/architecture/a-profile/docs/100933/latest/example-exception-handlers
.macro interrupt_handler handler:req,identify_and_clear_source
.type handler, %function
.type identify_and_clear_source, %function
    STR X30, [SP, #-8]!
    STP X28, X29, [SP, #-16]!
    STP X26, X27, [SP, #-16]!
    STP X24, X25, [SP, #-16]!
    STP X22, X23, [SP, #-16]!
    STP X20, X21, [SP, #-16]!
    STP X18, X19, [SP, #-16]!
    STP X16, X17, [SP, #-16]!
    STP X14, X15, [SP, #-16]!
    STP X12, X13, [SP, #-16]!
    STP X10, X11, [SP, #-16]!
    STP X8, X9, [SP, #-16]!
    STP X6, X7, [SP, #-16]!
    STP X4, X5, [SP, #-16]!
	STP X2, X3, [SP, #-16]!
    STP X0, X1, [SP, #-16]!

    MRS	X9, SPSR_EL1
	MRS	X10, ELR_EL1
    STP X9, X10, [SP, #-16]!
    // enable interrupts... (reentrant)

    .ifnb \identify_and_clear_source
        BL	\identify_and_clear_source
    .endif
	BL	\handler

    // disable interrupts... (reentrant)
    LDP X9, X10, [SP], #16
    MSR ELR_EL1, X10
    MSR SPSR_EL1, X9

	LDP X0, X1, [SP], #16
    LDP X2, X3, [SP], #16
	LDP X4, X5, [SP], #16
    LDP X6, X7, [SP], #16
	LDP X8, X9, [SP], #16
    LDP X10, X11, [SP], #16
	LDP X12, X13, [SP], #16
    LDP X14, X15, [SP], #16
	LDP X16, X17, [SP], #16
    LDP X18, X19, [SP], #16
	LDP X20, X21, [SP], #16
    LDP X22, X23, [SP], #16
	LDP X24, X25, [SP], #16
    LDP X26, X27, [SP], #16
	LDP X28, X29, [SP], #16
    LDR X30, [SP], #8
	ERET
.endm

.balign 0x800
.global vector_table_el1
vector_table_el1:
curr_el_sp0_sync:
    MSR SPSel, #0
    interrupt_handler _kernel_load _identify_sp0
.balign 0x80
curr_el_sp0_irq:
    MSR SPSel, #0
    interrupt_handler _kernel_load _identify_sp0
.balign 0x80
curr_el_sp0_fiq:
    MSR SPSel, #0
    interrupt_handler _kernel_load _identify_sp0
.balign 0x80
curr_el_sp0_serror:
    MSR SPSel, #0
    interrupt_handler test_handler

.balign 0x80
curr_el_spx_sync:
    interrupt_handler test_handler                  
.balign 0x80
curr_el_spx_irq:
    interrupt_handler test_handler
.balign 0x80
curr_el_spx_fiq:
    interrupt_handler test_handler
.balign 0x80
curr_el_spx_serror:
    interrupt_handler test_handler   

// These should be used when using USER mode
.balign 0x80
lower_el_aarch64_sync:
    b .
.balign 0x80
lower_el_aarch64_irq:
    b .
.balign 0x80
lower_el_aarch64_fiq:
    b .
.balign 0x80
lower_el_aarch64_serror:
    b .

.balign 0x80
lower_el_aarch32_sync:
    b .
.balign 0x80
lower_el_aarch32_irq:
    b .
.balign 0x80
lower_el_aarch32_fiq:
    b .
.balign 0x80
lower_el_aarch32_serror:
    b .
                         