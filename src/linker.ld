ENTRY(_start)

MEMORY
{
    RAM     (rwx)   : ORIGIN = 0x8000, LENGTH = 256M
    ROM     (rx)    : ORIGIN = 0x20000000, LENGTH = 512M
}

SECTIONS {
    __text_start = .;
    .init : {
        *(.init)
    } > RAM

    .text : {
        *(.text)
    } > RAM
    . = ALIGN(8);
    __text_end = .;

    __data_start = .;
    .data : {
        *(.rodata)
        *(.data)
    } > RAM
    . = ALIGN(8);
    __data_end = .;

    __bss_start = .;
    .bss : {
        *(.bss)
        *(COMMON)
    } > RAM
    . = ALIGN(8);
    __bss_end = .;

    __heap_start = .;
    . = . + 0x1000;
    . = ALIGN(8);
    __heap_end = .;

    __stack_start = .;
    . = . + 0x1000;    
    . = ALIGN(8);
    __stack_end = .;

    __irq_stack_start = .;
    . = . + 0x1000;    
    . = ALIGN(8);
    __irq_stack_end = .;
}