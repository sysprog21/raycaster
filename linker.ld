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

    .bss : {
        *(.bss)
    }

    . = ALIGN(16);
    __end = .;
}