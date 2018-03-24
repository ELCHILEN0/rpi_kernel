ENTRY(_init_core)

MEMORY
{
    /* Kernel boots at 0x8000 on 32 bit, 0x80000 on 64 bit.  Unless changed in config to 0x0. */
    RAM     (rwx)   : ORIGIN = 0x80000, LENGTH = 960M
    ROM     (rx)    : ORIGIN = 0x20000000, LENGTH = 1024M
}

SECTIONS {
    __text_start = .;
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

    .stack_core_0 : {
        . = ALIGN(8);
        __el0_stack_start_core_0 = .;
        . = . + 0x1000;    
        __el0_stack_end_core_0 = .;

        __el1_stack_start_core_0 = .;
        . = . + 0x1000;    
        __el1_stack_end_core_0 = .;
    } > RAM

    .stack_core_1 : {
        . = ALIGN(8);
        __el0_stack_start_core_1 = .;
        . = . + 0x1000;    
        __el0_stack_end_core_1 = .;

        __el1_stack_start_core_1 = .;
        . = . + 0x1000;    
        __el1_stack_end_core_1 = .;
    } > RAM

    .stack_core_2 : {
        . = ALIGN(8);
        __el0_stack_start_core_2 = .;
        . = . + 0x1000;    
        __el0_stack_end_core_2 = .;

        __el1_stack_start_core_2 = .;
        . = . + 0x1000;    
        __el1_stack_end_core_2 = .;
    } > RAM

    .stack_core_3 : {
        . = ALIGN(8);
        __el0_stack_start_core_3 = .;
        . = . + 0x1000;    
        __el0_stack_end_core_3 = .;

        __el1_stack_start_core_3 = .;
        . = . + 0x1000;    
        __el1_stack_end_core_3 = .;
    } > RAM


    __heap_start = .;
    . = . + 0x10000000;
    . = ALIGN(8);
    __heap_end = .;

    .note.gnu.build-id : {
        __build_id = .;
        *(.note.gnu.build-id);
    }

    /DISCARD/ : { }
}