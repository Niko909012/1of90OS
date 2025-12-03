OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
  /* Place the multiboot header at the start of the file so GRUB can find it
     within the first 8KiB. The rest of the kernel is linked to the 1MiB
     load address. */
  /* Try placing header at 0x400 to avoid large alignment to 0x2000 */
  /* Place header at file start (offset 0) */
  .multiboot : AT(0x0) { KEEP(*(.multiboot)) }
  . = 1M;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) *(COMMON) }
}
