ENTRY(_start)

SECTIONS {
    . = 0x8000;
    
    .text : {
        KEEP(*(.text.boot))
        *(.text)
    }

    .rodata :{
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }

    . = ALIGN(16);
    __end = .;
}