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

        *(.glue_7)
        *(.glue_7t)

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

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
    } > RAM

    . = ALIGN(4);

    end = .;
    _end = .;

    PROVIDE(end = .);
    PROVIDE(_end = .);

    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
