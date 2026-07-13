MEMORY
{
    text   (rx)   : ORIGIN = 0, LENGTH = 256K
    data   (rw!x) : ORIGIN = 0x800100, LENGTH = 8K
}

/* Use the default AVR linker script as a base */
INCLUDE "avr6.x"

/* Append our custom sections */
SECTIONS
{
    /* user-defined command list */
    .shell_cmds :
    {
        __start_shell_cmds = .;
        KEEP(*(.shell_cmds))
        __stop_shell_cmds = .;
    } > text

    /* user-defined flag list */
    .shell_flags :
    {
        __start_shell_flags = .;
        KEEP(*(.shell_flags))
        __stop_shell_flags = .;
    } > text

    /* Required by avr-gcc */
    .trampolines :
    {
        *(.trampolines)
    } > text
}
