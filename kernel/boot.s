/* Place multiboot header in the text segment so GRUB finds it */
.text
.align 4
.long 0x1BADB002
.long 0x00000000
.long -(0x1BADB002 + 0x00000000)

.global start
start:
    cli
    mov $stack_top, %esp
    call kernel_main
hang:
    hlt
    jmp hang

.bss
.align 16
stack:
    .space 0x4000
stack_top:
