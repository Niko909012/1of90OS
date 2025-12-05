# 1of90OS User Manual

## Version 1.0.0

---

## Table of Contents

1. [Introduction](#introduction)
2. [System Requirements](#system-requirements)
3. [Installation & Booting](#installation--booting)
4. [User Interface](#user-interface)
5. [Commands](#commands)
6. [Keyboard Layouts](#keyboard-layouts)
7. [Layout Selector Menu](#layout-selector-menu)
8. [Troubleshooting](#troubleshooting)
9. [Technical Specifications](#technical-specifications)

---

## Introduction

**1of90OS** is a minimal 32-bit x86 bootable kernel demonstrating core operating system concepts. It is written in Assembly (AT&T syntax) and C (freestanding) and boots via GRUB Multiboot v1.

### Key Features

- **32-bit x86 Architecture**: Bare-metal 32-bit protected mode kernel
- **Bootable ISO**: Creates a fully bootable ISO image (grub-mkrescue/xorriso)
- **VGA Text Mode**: 80×25 character display with color support
- **PS/2 Keyboard**: Polling-based keyboard input handling
- **5 International Keyboard Layouts**: US, DE, FR, IT, ES with special characters
- **Interactive Menu System**: Arrow-key navigable layout selector
- **Command-Line Interface**: Simple command dispatcher with built-in commands
- **No External Dependencies**: Uses only GCC, LD, and GRUB (freestanding C, no libc)

---

## System Requirements

### To Build 1of90OS

- **Linux environment** (tested on Ubuntu 24.04)
- **GCC with 32-bit support**: `gcc-multilib`
- **GRUB tools**: `grub-pc-bin` or equivalent
- **ISO creation tools**: `xorriso` (fallback from grub-mkrescue)
- **GNU Make** and **GNU LD**

### To Run 1of90OS

- **QEMU** with i386 CPU support (`qemu-system-i386`)
- **Or**: Physical x86 hardware capable of booting from CD/USB
- **Minimum RAM**: 32 MB (kernel is ~50 KB)

### To Build from Source

```bash
cd /workspaces/1of90OS
make clean
make iso/1of90os.iso
```

This generates `iso/1of90os.iso`.

### To Boot in QEMU

```bash
qemu-system-i386 -cdrom iso/1of90os.iso
```

Use `-nographic` for terminal output only (no graphical window).

---

## Installation & Booting

### From ISO (QEMU)

```bash
qemu-system-i386 -cdrom iso/1of90os.iso -m 256
```

You will see:
```
Welcome to 1of90OS v1.0.0!
Type 'help' for commands or 'manual' for user guide.

1of90OS[us]>
```

### From ISO (Physical Hardware)

1. Write ISO to USB or CD:
   ```bash
   # USB (replace sdX with your device)
   sudo dd if=iso/1of90os.iso of=/dev/sdX bs=4M && sync
   
   # Or use a tool like Etcher, YUMI, or Rufus
   ```

2. Boot from USB/CD in BIOS/UEFI
3. Select "1of90OS" from GRUB menu (auto-boots after 3 seconds)

---

## User Interface

### Prompt

```
1of90OS[layout]>
```

- **1of90OS**: System identifier
- **[layout]**: Current keyboard layout (us, de, fr, it, es)
- **>**: Input prompt

### Color Codes

- **White (0x07)**: Normal text and prompts
- **Yellow (0x0E)**: Layout names, important info
- **Green (0x0A)**: Selected menu items
- **Cyan (0x0F)**: Headers and banners
- **Red (0x0C)**: Error messages

### Text Input

- **Type character**: Character appears in input buffer
- **Backspace**: Deletes previous character
- **Shift**: Hold while typing for uppercase letters
- **Enter**: Execute command
- **Arrow keys**: Navigate layout menu (when open)

---

## Commands

### help

**Description**: Display list of all available commands

**Usage**:
```
1of90OS[us]> help
```

**Output**:
```
=== BUILT-IN COMMANDS ===
boot1of       - Display ASCII art logo
layout        - Open layout selection menu
ver           - Show system version
clear         - Clear the screen
help          - Show this help
info          - Show system information
manual        - Display user manual
=======================
```

---

### ver (or version)

**Description**: Display system version and build information

**Usage**:
```
1of90OS[us]> ver
```

**Output**:
```
1of90OS v1.0.0
32-bit x86 Bootable Kernel
Built with Assembly + C (freestanding)
Features: PS/2 Keyboard, 5 Layouts, VGA Text Mode
```

---

### info

**Description**: Show detailed system information

**Usage**:
```
1of90OS[us]> info
```

**Output**:
```
=== SYSTEM INFORMATION ===
Platform:     32-bit x86 (i386)
Boot Method:  GRUB Multiboot v1
Display:      VGA Text Mode (80x25)
Keyboard:     PS/2 Polling
Layouts:      5 (US, DE, FR, IT, ES)
Memory:       32-bit flat model
==========================
```

---

### clear

**Description**: Clear the screen and reset cursor to top-left

**Usage**:
```
1of90OS[us]> clear
```

---

### layout

**Description**: Open interactive keyboard layout selector menu

**Usage**:
```
1of90OS[us]> layout
```

**Menu Interface**:
```
=======================================
        SELECT KEYBOARD LAYOUT
=======================================

>>> US QWERTY
    DE QWERTZ
    FR AZERTY
    IT QWERTY
    ES QWERTY

Use UP/DOWN arrows to select, ENTER to confirm
```

- **UP Arrow**: Move to previous layout
- **DOWN Arrow**: Move to next layout
- **ENTER**: Confirm selection and close menu
- **Any other key during menu**: No effect (ignored)

---

### boot1of

**Description**: Display ASCII art logo

**Usage**:
```
1of90OS[us]> boot1of
```

**Output**:
```
+------------------------------+
|                              |
|        HELLO WORLD!          |
|                              |
+------------------------------+
```

---

### manual

**Description**: Display full user manual (interactive)

**Usage**:
```
1of90OS[us]> manual
```

The manual displays in a paged format. Press any key to return to prompt.

---

## Keyboard Layouts

1of90OS supports 5 international keyboard layouts with CP-437 character encoding for proper VGA text mode display.

### US QWERTY (Default)

Standard US QWERTY layout.

```
Row 1: 1 2 3 4 5 6 7 8 9 0 - =
Row 2: Q W E R T Y U I O P [ ]
Row 3: A S D F G H J K L ; '
Row 4: Z X C V B N M , . /
```

---

### DE QWERTZ (German)

QWERTZ layout with German special characters.

**Umlauts**:
- `Shift+a` → Ä
- `[` key → Ü (ü)
- `;` key → Ö (ö)
- `=` key → ß (sharp S)

**Y/Z Swapped**: German keyboards have Z and Y swapped compared to US layout.

```
Row 1: 1 2 3 4 5 6 7 8 9 0 ß =
Row 2: Q W E R T Z U I O P Ü +
Row 3: A S D F G H J K L Ö Ä ^
Row 4: Y X C V B N M , . /
```

**Character Map**:
- ä (a-umlaut): CP-437 code 0x84
- ö (o-umlaut): CP-437 code 0x94
- ü (u-umlaut): CP-437 code 0x81
- ß (sharp S): CP-437 code 0xE1

---

### FR AZERTY (French)

AZERTY layout with French accents.

**Accents**:
- `2` key → é (e-acute)
- `8` key → è (e-grave)
- `3` key → ê (e-circumflex)
- `0` key → ç (c-cedilla)

```
Row 1: & é " ' ( - è _ ç a ) =
Row 2: A Z E R T Y U I O P [ ]
Row 3: Q S D F G H J K L M U `
Row 4: W X C V B N , ; : /
```

**Character Map**:
- é (e-acute): CP-437 code 0x82
- è (e-grave): CP-437 code 0x8A
- ê (e-circumflex): CP-437 code 0x88
- ç (c-cedilla): CP-437 code 0x9F

---

### IT QWERTY (Italian)

Italian QWERTY with Italian accents.

**Accents**:
- `[` key → è (e-grave)
- `;` key → ò (o-grave)
- `'` key → à (a-grave)

```
Row 1: 1 2 3 4 5 6 7 8 9 0 - =
Row 2: Q W E R T Y U I O P è +
Row 3: A S D F G H J K L ò à `
Row 4: Z X C V B N M , . /
```

**Character Map**:
- à (a-grave): CP-437 code 0x85
- è (e-grave): CP-437 code 0x8A
- ò (o-grave): CP-437 code 0x95

---

### ES QWERTY (Spanish)

Spanish QWERTY with Spanish accents and special characters.

**Accents**:
- `A` key → á (a-acute)
- `3` key → é (e-acute)
- `;` key → ñ (n-tilde)

```
Row 1: 1 2 3 4 5 6 7 8 9 0 - =
Row 2: Q W é R T Y U I O P [ ]
Row 3: á S D F G H J K L ñ ' `
Row 4: Z X C V B N M , . /
```

**Character Map**:
- á (a-acute): CP-437 code 0xA0
- é (e-acute): CP-437 code 0x82
- ñ (n-tilde): CP-437 code 0xA4

---

## Layout Selector Menu

### Opening the Menu

Type `layout` at the command prompt and press ENTER:

```
1of90OS[us]> layout
```

### Menu Navigation

The layout menu appears with the current layout highlighted in green:

```
=======================================
        SELECT KEYBOARD LAYOUT
=======================================

>>> US QWERTY              (green, selected)
    DE QWERTZ
    FR AZERTY
    IT QWERTY
    ES QWERTY

Use UP/DOWN arrows to select, ENTER to confirm
```

### Selection Process

1. **UP Arrow** (`↑`): Move selection up (wraps to bottom)
2. **DOWN Arrow** (`↓`): Move selection down (wraps to top)
3. **ENTER**: Confirm selection and return to prompt

### Example Session

```
1of90OS[us]> layout
[Menu opens showing US highlighted]

[User presses DOWN arrow]
[Menu updates: DE now highlighted in green]

[User presses DOWN arrow again]
[Menu updates: FR now highlighted]

[User presses ENTER]
[Menu closes, returns to prompt with new layout]

1of90OS[fr]>
```

---

## Troubleshooting

### Keyboard Not Responding

**Symptoms**: No characters appear when typing

**Solutions**:
1. Enable PS/2 keyboard in QEMU: `-machine pc` (default)
2. Ensure keyboard focus is on QEMU window (click window)
3. Wait 1-2 seconds after boot for keyboard controller to initialize
4. Try pressing a simple key first (like space)

### Special Characters Not Displaying

**Symptoms**: Umlauts/accents show as blank or wrong characters

**Cause**: VGA text mode uses CP-437 encoding, not Unicode

**Solution**: Characters are hard-coded to CP-437 values and should display correctly. If not:
1. Verify terminal supports CP-437 (most do)
2. Try different terminal/emulator
3. Recompile kernel (some platforms may have font issues)

### Layout Selector Menu Unresponsive

**Symptoms**: Arrow keys don't navigate menu

**Cause**: PS/2 extended key codes not detected

**Solution**:
1. Use standard keyboard (laptop keyboards may have issues)
2. Ensure arrow keys work in normal input (test by typing, then backspace)
3. Try in QEMU with full keyboard emulation: `-usb -usbdevice keyboard`

### System Hangs After Boot

**Symptoms**: Kernel loads but hangs before prompt

**Cause**: Keyboard polling loop or PS/2 controller issue

**Solution**:
1. Add `-usb` to QEMU for better keyboard support
2. Compile with debug symbols (already enabled in Makefile: `-g`)
3. Try with older CPU model: `-cpu 486`

---

## Technical Specifications

### Architecture

- **CPU**: 32-bit x86 (Intel i386 and compatible)
- **Mode**: Protected mode (no real mode, no long mode)
- **Entry Point**: Multiboot v1 bootloader (GRUB)
- **Base Address**: 0x100000 (1 MiB)

### Memory Layout

```
0x00000000  +---------------------------+
            | BIOS/IVT (reserved)       |
0x00100000  +---------------------------+
            | Kernel (kernel.bin)       | <- 0x100000 (entry point)
            | Stack (16 KB)             |
            | Code, Data, BSS           |
0x00104000  +---------------------------+
            | Free memory               |
            |                           |
0xFFFFFFFF  +---------------------------+
```

### Boot Process

1. BIOS/UEFI loads GRUB from ISO
2. GRUB parses `grub.cfg` and loads `/boot/kernel.bin`
3. GRUB passes Multiboot header info to kernel
4. Kernel entry point (`kernel/boot.s`, symbol `start`) runs
5. Stack initialized, interrupts disabled
6. Calls `kernel_main()` (C function)

### VGA Text Mode

- **Base Address**: 0xB8000 (linear frame buffer)
- **Resolution**: 80 columns × 25 rows
- **Display Memory**: 4 KB (80 × 25 × 2 bytes)
- **Format**: Each cell = 2 bytes
  - Byte 0: Character (ASCII/CP-437)
  - Byte 1: Attribute (foreground color << 0, background color << 4)
- **Colors**: 16 colors (0x0 to 0xF), including bright variants

### Keyboard (PS/2)

- **Port 0x60**: Data port (read scancodes)
- **Port 0x64**: Controller status port (bit 0 = output buffer full)
- **Protocol**: Scancode set 1 (XT-compatible)
- **Special Keys**:
  - Extended keys (0xE0 prefix): arrow keys, etc.
  - Released keys (high bit set): 0x80 + scancode
  - Shift keys: 0x2A (left), 0x36 (right)

### Kernel Size

- **kernel.bin**: ~50 KB (varies with optimization)
- **ISO**: ~2.5 MB (includes GRUB)

### Limitations

- **No Interrupts**: Keyboard is polled, not interrupt-driven
- **No Paging**: Direct 1:1 physical-to-virtual mapping
- **No Filesystem**: No disk I/O or file reading
- **No Multitasking**: Single-threaded busy-wait loop
- **No Protected Memory**: All code runs in ring 0
- **No Floating Point**: No FPU/SSE support
- **Console Only**: VGA text mode, no graphics

---

## Building from Source

### Prerequisites

```bash
sudo apt install gcc-multilib grub-pc-bin xorriso make
```

### Compilation

```bash
cd /workspaces/1of90OS
make clean          # Remove old build artifacts
make kernel.bin     # Compile kernel only
make iso/1of90os.iso # Generate bootable ISO
```

### Makefile Targets

| Target | Purpose |
|--------|---------|
| `make kernel.bin` | Compile kernel.boot + kernel/main.c → kernel.bin |
| `make iso/1of90os.iso` | Generate ISO (requires kernel.bin) |
| `make clean` | Remove build artifacts |

### Output Files

```
iso/1of90os.iso         # Bootable ISO image
kernel.bin              # Compiled kernel binary
boot.o, main.o          # Object files
```

---

## Source Code Structure

```
/workspaces/1of90OS/
├── kernel/
│   ├── boot.s           # Assembly bootloader (Multiboot header + entry)
│   └── main.c           # Main kernel code (VGA, keyboard, commands)
├── linker.ld            # Custom linker script (32-bit ELF)
├── Makefile             # Build configuration
├── grub.cfg             # GRUB boot menu configuration
├── scripts/
│   └── mkiso.sh         # ISO creation helper script
├── iso/
│   └── 1of90os.iso      # Generated bootable ISO
├── OS_README.md         # Quick build guide
└── USER_MANUAL.md       # This file
```

---

## Development Notes

### Adding New Commands

Edit `kernel/main.c`, add function:

```c
static void cmd_mycommand(void) {
    puts_col("Output here\n", 0x07);
}
```

Then in `kernel_main()` command dispatcher:

```c
} else if (mystrcmp(buf, "mycommand") == 0) {
    cmd_mycommand();
```

And update `cmd_help()` with description.

### Adding New Keyboard Layouts

1. Create new scancode map array:
```c
static const unsigned char scancode_xx[128] = {
    [0x02] = '1', [0x03] = '2', ...
};
```

2. Add to layout enum:
```c
typedef enum { ..., LAYOUT_XX, ... } layout_t;
```

3. Update `map_scancode()`:
```c
else if (current_layout == LAYOUT_XX) map = scancode_xx;
```

4. Update layout menu array in `show_layout_menu()`

### Compiling with Optimizations

Change `-O0` to `-O2` or `-O3` in Makefile for smaller/faster kernel.

---

## References

- **Multiboot v1 Specification**: https://www.gnu.org/software/grub/manual/multiboot/
- **x86 Assembly**: AT&T syntax (GCC inline asm)
- **PS/2 Protocol**: XT scancode set 1
- **VGA Text Mode**: Standard IBM VGA (0xB8000)
- **CP-437 Encoding**: DOS/VGA character set

---

## License & Credits

1of90OS is a minimal educational kernel created as a demonstration of x86 OS fundamentals. It is provided as-is for learning purposes.

**Built with**:
- GCC (GNU Compiler Collection)
- GNU LD (Linker)
- GRUB (Bootloader)
- xorriso (ISO creation)

---

**Last Updated**: December 2025  
**Version**: 1.0.0
