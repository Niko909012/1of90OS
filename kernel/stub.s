.section .multiboot
.align 4
.long 0x1BADB002
.long 0x00000000
.long -(0x1BADB002 + 0x00000000)

.text
.global start
start:
    cli
    /* set up stack */
    mov $stack_top, %esp

    /* debug: write 'S' to top-left of VGA to indicate stub started */
    mov $0x1F53, %ax    /* 'S' with attribute 0x1F */
    mov %ax, 0xb8000

    /* EBX contains pointer to multiboot_info */
    mov %ebx, %esi

    /* read mods_count (offset 0x14) and mods_addr (offset 0x18) */
    mov 0x14(%esi), %ecx    /* mods_count */
    test %ecx, %ecx
    jz no_modules
    mov 0x18(%esi), %ebx    /* mods_addr */

    /* take first module */
    /* take first module and jump to its start (GRUB already loaded it) */
    mov (%ebx), %eax        /* mod_start */
    /* jump to module start (address in %eax) */
    jmp *%eax

no_modules:
    /* hang */
    hlt
    jmp .

.bss
.align 16
stack:
    .space 0x4000
stack_top:
