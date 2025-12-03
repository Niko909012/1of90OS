1of90OS — Minimal kernel base (assembly + C)

Overview
- A tiny Multiboot v1-compatible kernel (32-bit) written in assembly + C.
- Produces a bootable ISO that GRUB can load. The kernel prints a welcome message on VGA text mode.

Files created
- `kernel/boot.s` — multiboot header + simple entry (assembly)
- `kernel/main.c` — tiny C kernel that writes to VGA text buffer
- `linker.ld` — linker script (links at 1MiB)
- `grub.cfg` — GRUB menu entry
- `Makefile` — build pipeline: compile, link, and create ISO

Dependencies (Ubuntu / Debian)
Run:

```bash
sudo apt update
sudo apt install build-essential gcc-multilib binutils-multiarch xorriso grub-pc-bin qemu-system-x86
```

Notes:
- `gcc-multilib` and related 32-bit toolchain packages are required because the kernel is built as 32-bit ELF.
- `grub-mkrescue` comes from `grub-pc-bin` and needs `xorriso` to create ISO images.

Build

From repository root:

```bash
make
```

If successful, the ISO will be at `iso/1of90os.iso`.

Run in QEMU

```bash
qemu-system-i386 -cdrom iso/1of90os.iso -m 64M
```

Next steps (ideas)
- Add VGA text cursor + basic input (read keyboard scancodes via port 0x60)
- Implement simple DOS-like filesystem (FAT) or command parser
- Add drivers for disk/serial

If you want, I can:
- Try running `make` here and report errors (I may lack packages in the container), or
- Convert the build to x86_64 / Multiboot2 if you prefer 64-bit booting.
