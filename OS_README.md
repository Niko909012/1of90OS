# 1of90OS - Complete Kernel & User Manual

## Overview

**1of90OS** is a complete minimal 32-bit x86 bootable kernel demonstrating core operating system concepts. It includes:

- **32-bit Protected Mode Kernel** (Assembly + C)
- **Bootable GRUB Multiboot v1 ISO**
- **VGA Text Mode Display** (80×25 colors)
- **PS/2 Keyboard Input** (Polling-based)
- **5 International Keyboard Layouts** (US, DE, FR, IT, ES)
- **Interactive Command Interface**
- **ASCII Menu System** (Arrow-key navigation)
- **Built-in Help & User Manual**

## Quick Start

### Build

```bash
cd /workspaces/1of90OS
make clean
make iso/1of90os.iso
```

### Run in QEMU

```bash
qemu-system-i386 -cdrom iso/1of90os.iso -m 256
```

## System Requirements

### To Build

- Linux (Ubuntu 24.04+)
- `gcc-multilib`, `grub-pc-bin`, `xorriso`, `make`

Install on Ubuntu/Debian:

```bash
sudo apt install gcc-multilib grub-pc-bin xorriso build-essential
```

### To Run

- QEMU: `sudo apt install qemu-system-x86`
- Or: Physical x86 hardware (Pentium+) with CD/USB boot

## Commands

| Command | Description |
|---------|-------------|
| `help` | Show all commands |
| `ver` | Version info (1.0.0) |
| `info` | System information |
| `manual` | Full user manual |
| `clear` | Clear screen |
| `layout` | Select keyboard layout (interactive menu) |
| `boot1of` | Display ASCII art logo |

## Keyboard Layouts

- **US** — QWERTY (standard US)
- **DE** — QWERTZ (German: ä ö ü ß)
- **FR** — AZERTY (French: é è ê ç)
- **IT** — QWERTY (Italian: à è ò)
- **ES** — QWERTY (Spanish: á é ñ)

### Change Layout

```
1of90OS[us]> layout
```

Menu opens. Use UP/DOWN arrows to select, ENTER to confirm.

## Files

```
kernel/
├── boot.s          Assembly entry point + Multiboot header
└── main.c          Complete kernel (VGA, keyboard, commands)
linker.ld           32-bit ELF linker script
Makefile            Build automation
grub.cfg            GRUB boot menu
scripts/mkiso.sh    ISO creation helper
iso/1of90os.iso     Generated bootable ISO
USER_MANUAL.md      Complete user documentation
OS_README.md        This file
```

## Full Documentation

**For complete documentation, see `USER_MANUAL.md`** which includes:

- Detailed command reference
- Keyboard layout specifications
- Layout selector menu guide
- Troubleshooting
- Technical specifications
- Building & development guide
- Source code structure

## Features Implemented

✅ Multiboot v1 bootloader support
✅ 32-bit protected mode kernel
✅ VGA text mode (80×25 with colors)
✅ PS/2 keyboard polling
✅ 5 keyboard layouts with international characters
✅ Interactive ASCII menu (arrow keys)
✅ Command dispatcher
✅ System version & info commands
✅ Help system
✅ User manual (built-in)
✅ Clear screen
✅ ASCII art logo
✅ Backspace & shift support
✅ Modular C code

## Kernel Features

### VGA Display

- 80×25 character display
- 16 colors (0x0-0xF)
- Color attributes (foreground/background)
- Automatic scrolling
- Cursor tracking

### Keyboard

- PS/2 polling (no interrupts)
- Scancode set 1 (XT-compatible)
- Shift key detection
- Extended keys (arrow keys)
- 5 international layouts
- Character mapping via CP-437

### Command Interface

- Prompt shows current layout
- Input buffer with backspace
- Command dispatcher
- 7 built-in commands
- Error messages

## Development

### Adding New Commands

Edit `kernel/main.c`:

```c
static void cmd_mycommand(void) {
    puts_col("My output\n", 0x07);
}
```

Add to dispatcher in `kernel_main()`:

```c
} else if (mystrcmp(buf, "mycommand") == 0) {
    cmd_mycommand();
```

### Adding New Keyboard Layout

1. Create scancode array
2. Add to layout enum
3. Update `map_scancode()` switch
4. Update menu in `show_layout_menu()`

### Compilation Options

Change optimization in Makefile:

```makefile
# From: -O0 (no optimization)
# To: -O2 or -O3 (smaller/faster)
```

## Boot Sequence

1. BIOS/UEFI loads GRUB from ISO
2. GRUB displays menu (auto-boots in 3s)
3. GRUB loads `/boot/kernel.bin`
4. `kernel/boot.s` entry point:
   - Verify Multiboot magic
   - Set up 16 KB stack
   - Disable interrupts
   - Call `kernel_main()`
5. `kernel_main()` (C):
   - Initialize VGA display
   - Enable PS/2 keyboard
   - Show welcome message
   - Enter command loop

## Technical Specs

| Aspect | Details |
|--------|---------|
| **Architecture** | 32-bit x86 (i386) |
| **Mode** | Protected mode |
| **Boot** | GRUB Multiboot v1 |
| **Load Address** | 0x100000 (1 MiB) |
| **Display** | VGA text mode (0xB8000) |
| **Keyboard** | PS/2 polling |
| **Memory** | Flat 32-bit address space |
| **Kernel Size** | ~50 KB |
| **ISO Size** | ~2.5 MB |

## Troubleshooting

### Keyboard Not Working

1. Wait 1-2 seconds after boot
2. Click QEMU window for focus
3. Try: `qemu-system-i386 ... -usb -usbdevice keyboard`

### Layout Menu Unresponsive

- Use numeric keypad arrow keys
- Try different keyboard/terminal

### Characters Display Wrong

- VGA uses CP-437 encoding
- Should work on all modern terminals
- If not, try recompiling: `make clean && make iso/1of90os.iso`

## References

- **Multiboot v1**: https://www.gnu.org/software/grub/manual/multiboot/
- **x86 Assembly**: AT&T syntax for inline asm
- **PS/2 Protocol**: XT scancode set 1
- **VGA Text Mode**: 0xB8000 memory-mapped
- **CP-437**: DOS/VGA character encoding

## Building on Different Systems

### Ubuntu/Debian

```bash
sudo apt install gcc-multilib grub-pc-bin xorriso make
cd /workspaces/1of90OS && make iso/1of90os.iso
```

### Fedora/RHEL

```bash
sudo dnf install gcc gcc-c++ glibc-devel.i686 grub2-tools xorriso
```

### macOS/WSL

May require cross-compiler setup. Recommended: use Linux VM or container.

## Installation on USB

```bash
# Identify USB device
lsblk

# Write ISO (replace sdX with your USB device)
sudo dd if=iso/1of90os.iso of=/dev/sdX bs=4M status=progress && sync

# Or use Etcher, YUMI, or Rufus on Windows
```

Boot from USB in BIOS/UEFI settings.

## Source Code Highlights

### boot.s (Assembly)

- Multiboot header magic (0x1BADB002)
- GDT basic setup
- 16 KB stack allocation
- Jump to `kernel_main()`

### main.c (C)

- VGA I/O functions (`putchar_col`, `puts_col`)
- PS/2 keyboard polling (`get_char_poll`)
- Scancode mapping (5 layouts)
- Command parser
- 7 built-in commands
- Layout selector menu

## Performance

- Boot time: ~3-5 seconds (including GRUB)
- Command response: Instant (no delays)
- Keyboard latency: ~10-50ms (polling interval)
- Memory usage: ~1 MB (kernel + stack + video)

## Limitations

- No interrupts (polling only)
- No paging (identity mapping)
- No filesystem
- No multitasking
- Text mode only
- Ring 0 (no memory protection)
- No floating point

## Future Enhancements

- Interrupt-driven keyboard
- Disk I/O and FAT filesystem
- Paging and virtual memory
- Multitasking scheduler
- Serial port logging
- Graphics mode
- More commands

## License & Credit

Educational kernel for learning x86 OS fundamentals. Built with GCC, GNU LD, and GRUB.

---

**Version**: 1.0.0
**Last Updated**: December 2025
**Documentation**: See `USER_MANUAL.md` for complete guide
