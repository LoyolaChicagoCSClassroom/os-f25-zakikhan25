ENTRY(main)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        _start_data = .;
        *(.data)
        _end_data = .;
    }

    .bss : {
        _start_bss = .;
        *(.bss)
        _end_bss = .;
    }

    .stack : {
        _start_stack = .;
        *(.stack)
        _end_stack = .;
    }

    _end_kernel = .;
}
