OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
    /* QEMU "raspi2" emulation may incorrectly load the kernel binaries at 0x10000 instead of 0x8000, so if you do not see any output, try adjusting the base address constant in the linker script.  */
    RAM     (rwx)   : ORIGIN = 0x80000, LENGTH = 0x8000000
    /* ROM     (rx)    : ORIGIN = 0x20000000, LENGTH = 512M */
}

SECTIONS
{
    __text_start = .;
    .text : {
        *(.text)
    } > RAM
    . = ALIGN(4096);
    __text_end = .;

    __data_start = .;
    .data : {
        *(.rodata)
        *(.data)
    } > RAM
    . = ALIGN(4096);
    __data_end = .;

    __bss_start = .;
    .bss : {
        *(.bss)
        *(COMMON)
    } > RAM
    . = ALIGN(4096);
    __bss_end = .;

    __heap_start = .;
    . = . + 0x10000;
    __heap_end = .;

    __stack_start = .;
    . = . + 0x10000;
    __stack_end = .;

    __irq_stack_start = .;
    . = . + 0x10000;
    __irq_stack_end = .;
}
