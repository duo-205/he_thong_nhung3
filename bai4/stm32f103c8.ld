MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        . = ALIGN(4);

        KEEP(*(.isr_vector))

        *(.text*)
        *(.rodata*)

        . = ALIGN(4);

        *(.ARM.extab*)
        *(.ARM.exidx*)

        . = ALIGN(4);

        _etext = .;
    } > FLASH


    .data :
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data*)

        . = ALIGN(4);

        _edata = .;

    } > RAM AT > FLASH

    _sidata = LOADADDR(.data);


    .bss :
    {
        . = ALIGN(4);

        _sbss = .;
        __bss_start__ = .;

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
        __bss_end__ = .;

    } > RAM


    . = ALIGN(4);

    end = .;
    _end = .;


    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
