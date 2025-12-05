#include <stdint.h>

// ============================================================================
// VGA TEXT MODE I/O
// ============================================================================

static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    asm volatile ("inb %1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

volatile uint16_t *vga = (volatile uint16_t*)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

static void putchar_col(unsigned char c, uint8_t color) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        return;
    }
    if (c == '\r') return;
    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            int idx = cursor_row * 80 + cursor_col;
            vga[idx] = (uint16_t)(' ') | (color << 8);
        }
        return;
    }
    if (cursor_col >= 80) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= 25) {
        // scroll up by one line
        for (int r = 1; r < 25; ++r) {
            for (int cc = 0; cc < 80; ++cc) {
                vga[(r-1)*80 + cc] = vga[r*80 + cc];
            }
        }
        // clear last line
        for (int cc = 0; cc < 80; ++cc) vga[24*80 + cc] = (uint16_t)(' ') | (0x07 << 8);
        cursor_row = 24;
    }
    int idx = cursor_row * 80 + cursor_col;
    vga[idx] = (uint16_t)c | (color << 8);
    cursor_col++;
}

static void puts_col(const char *s, uint8_t color) {
    for (int i = 0; s[i]; ++i) putchar_col(s[i], color);
}

static void clear_screen(void) {
    for (int i = 0; i < 80*25; ++i) vga[i] = (uint16_t)(' ') | (0x07 << 8);
    cursor_row = 0;
    cursor_col = 0;
}

static void gotoxy(int row, int col) {
    cursor_row = row;
    cursor_col = col;
}

// ============================================================================
// KEYBOARD & LAYOUT SYSTEM
// ============================================================================

typedef enum { LAYOUT_US = 0, LAYOUT_DE = 1, LAYOUT_FR = 2, LAYOUT_IT = 3, LAYOUT_ES = 4 } layout_t;
static layout_t current_layout = LAYOUT_US;
static const char *layout_name = "us";
static int shift_pressed = 0;

// US QWERTY
static const unsigned char scancode_us[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x39] = ' '
};

// German QWERTZ with umlauts (CP-437)
static const unsigned char scancode_de[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = 0xE1, /* ß */ [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'z', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = 0x81, /* ü */ [0x1B] = '+', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = 0x94, /* ö */
    [0x28] = 0x84, /* ä */ [0x29] = '^', [0x2B] = '#',
    [0x2C] = 'y', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '/', [0x39] = ' '
};

// French AZERTY
static const unsigned char scancode_fr[128] = {
    [0x02] = '&', [0x03] = 0x82, /* é */ [0x04] = '"', [0x05] = '\'', [0x06] = '(',
    [0x07] = '-', [0x08] = 0x8A, /* è */ [0x09] = '_', [0x0A] = 0x9F, /* ç */ [0x0B] = 'a',
    [0x0C] = ')', [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'a', [0x11] = 'z', [0x12] = 0x88, /* ê */ [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'q', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = 'm',
    [0x28] = 'u', [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'w', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = ',', [0x33] = ';', [0x34] = ':',
    [0x35] = '/', [0x39] = ' '
};

// Italian QWERTY
static const unsigned char scancode_it[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = 0x8A, /* è */ [0x1B] = '+', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = 0x95, /* ò */
    [0x28] = 0x85, /* à */ [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '/', [0x39] = ' '
};

// Spanish QWERTY
static const unsigned char scancode_es[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'q', [0x11] = 'w', [0x12] = 0x82, /* é */ [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 0xA0, /* á */ [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = 0xA4, /* ñ */
    [0x28] = '\'', [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '/', [0x39] = ' '
};

static int mystrcmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return (int)(a[i] - b[i]);
        ++i;
    }
    return (int)(a[i] - b[i]);
}

static int starts_with(const char *s, const char *pre) {
    int i = 0;
    while (pre[i]) {
        if (s[i] != pre[i]) return 0;
        i++;
    }
    return 1;
}

static void set_layout_by_index(int idx) {
    switch (idx) {
        case 0: current_layout = LAYOUT_US; layout_name = "us"; break;
        case 1: current_layout = LAYOUT_DE; layout_name = "de"; break;
        case 2: current_layout = LAYOUT_FR; layout_name = "fr"; break;
        case 3: current_layout = LAYOUT_IT; layout_name = "it"; break;
        case 4: current_layout = LAYOUT_ES; layout_name = "es"; break;
    }
}

static unsigned char map_scancode(unsigned char sc) {
    const unsigned char *map = scancode_us;
    if (current_layout == LAYOUT_DE) map = scancode_de;
    else if (current_layout == LAYOUT_FR) map = scancode_fr;
    else if (current_layout == LAYOUT_IT) map = scancode_it;
    else if (current_layout == LAYOUT_ES) map = scancode_es;
    
    if (sc >= 128) return 0;
    unsigned char base = map[sc];
    if (!base) return 0;
    if (shift_pressed && base >= 'a' && base <= 'z') return (unsigned char)(base - 'a' + 'A');
    return base;
}

// Scancode conversion: up/down/left/right = 0x48/0x50/0x4B/0x4D (extended, 0xE0 prefix)
static int last_extended = 0;
static unsigned char get_char_poll(void) {
    if ((inb(0x64) & 1) == 0) return 0;
    unsigned char sc = inb(0x60);
    
    // Extended key prefix (0xE0)
    if (sc == 0xE0) {
        last_extended = 1;
        return 0;
    }
    
    unsigned char code = sc & 0x7F;
    int released = sc & 0x80;

    // Shift keys
    if (code == 0x2A || code == 0x36) {
        if (released) shift_pressed = 0; else shift_pressed = 1;
        return 0;
    }

    // Arrow keys (extended)
    if (last_extended && !released) {
        last_extended = 0;
        switch (code) {
            case 0x48: return 0x18; // UP
            case 0x50: return 0x19; // DOWN
            case 0x4B: return 0x1A; // LEFT
            case 0x4D: return 0x1B; // RIGHT
            default: return 0;
        }
    }
    last_extended = 0;

    if (released) return 0;
    return map_scancode(code);
}

// ============================================================================
// LAYOUT SELECTION MENU
// ============================================================================

static void show_layout_menu(void) {
    clear_screen();
    gotoxy(0, 0);
    puts_col("=======================================\n", 0x0F);
    puts_col("        SELECT KEYBOARD LAYOUT\n", 0x0F);
    puts_col("=======================================\n", 0x0F);
    puts_col("\n", 0x07);
    
    static const char *layouts[5] = { "US QWERTY", "DE QWERTZ", "FR AZERTY", "IT QWERTY", "ES QWERTY" };
    int selected = (int)current_layout;
    
    for (int i = 0; i < 5; ++i) {
        if (i == selected) {
            puts_col(">>> ", 0x0A); // Green (selected)
        } else {
            puts_col("    ", 0x07);
        }
        puts_col(layouts[i], (i == selected) ? 0x0A : 0x07);
        puts_col("\n", 0x07);
    }
    
    puts_col("\n", 0x07);
    puts_col("Use UP/DOWN arrows to select, ENTER to confirm\n", 0x0E);
}

static void layout_menu_loop(void) {
    show_layout_menu();
    
    for (;;) {
        unsigned char c = get_char_poll();
        if (c == 0) {
            asm volatile ("pause");
            continue;
        }
        
        if (c == 0x18) { // UP
            int idx = (int)current_layout;
            if (idx > 0) set_layout_by_index(idx - 1);
            show_layout_menu();
        } else if (c == 0x19) { // DOWN
            int idx = (int)current_layout;
            if (idx < 4) set_layout_by_index(idx + 1);
            show_layout_menu();
        } else if (c == '\n') { // ENTER
            break;
        }
    }
    
    clear_screen();
    gotoxy(0, 0);
}

// ============================================================================
// BUILT-IN HELP & VERSION
// ============================================================================

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

static void print_prompt(void) {
    cursor_col = 0;
    puts_col("1of90OS[", 0x07);
    puts_col(layout_name, 0x0E);
    puts_col("]> ", 0x07);
}

static void cmd_help(void) {
    puts_col("\n=== BUILT-IN COMMANDS ===\n", 0x0F);
    puts_col("boot1of       - Display ASCII art logo\n", 0x07);
    puts_col("layout        - Open layout selection menu\n", 0x07);
    puts_col("ver           - Show system version\n", 0x07);
    puts_col("clear         - Clear the screen\n", 0x07);
    puts_col("help          - Show this help\n", 0x07);
    puts_col("info          - Show system information\n", 0x07);
    puts_col("manual        - Display user manual\n", 0x07);
    puts_col("=======================\n\n", 0x0F);
}

static void cmd_version(void) {
    puts_col("\n1of90OS v", 0x0F);
    putchar_col('0' + VERSION_MAJOR, 0x0F);
    putchar_col('.', 0x0F);
    putchar_col('0' + VERSION_MINOR, 0x0F);
    putchar_col('.', 0x0F);
    putchar_col('0' + VERSION_PATCH, 0x0F);
    puts_col("\n", 0x0F);
    puts_col("32-bit x86 Bootable Kernel\n", 0x07);
    puts_col("Built with Assembly + C (freestanding)\n", 0x07);
    puts_col("Features: PS/2 Keyboard, 5 Layouts, VGA Text Mode\n\n", 0x07);
}

static void cmd_info(void) {
    puts_col("\n=== SYSTEM INFORMATION ===\n", 0x0F);
    puts_col("Platform:     32-bit x86 (i386)\n", 0x07);
    puts_col("Boot Method:  GRUB Multiboot v1\n", 0x07);
    puts_col("Display:      VGA Text Mode (80x25)\n", 0x07);
    puts_col("Keyboard:     PS/2 Polling\n", 0x07);
    puts_col("Layouts:      5 (US, DE, FR, IT, ES)\n", 0x07);
    puts_col("Memory:       32-bit flat model\n", 0x07);
    puts_col("==========================\n\n", 0x0F);
}

static void cmd_manual(void) {
    clear_screen();
    gotoxy(0, 0);
    puts_col("=== 1of90OS USER MANUAL ===\n\n", 0x0F);
    puts_col("1. INTRODUCTION\n", 0x0F);
    puts_col("   1of90OS is a minimal bootable x86 kernel written in\n", 0x07);
    puts_col("   Assembly and C. It demonstrates core OS concepts:\n", 0x07);
    puts_col("   - Bootloader integration (GRUB Multiboot)\n", 0x07);
    puts_col("   - VGA text-mode display\n", 0x07);
    puts_col("   - PS/2 keyboard input polling\n", 0x07);
    puts_col("   - International keyboard layout support\n\n", 0x07);
    
    puts_col("2. KEYBOARD LAYOUTS\n", 0x0F);
    puts_col("   5 layouts are supported:\n", 0x07);
    puts_col("   - US QWERTY (default)\n", 0x07);
    puts_col("   - DE QWERTZ (German with ä, ö, ü, ß)\n", 0x07);
    puts_col("   - FR AZERTY (French with accents)\n", 0x07);
    puts_col("   - IT QWERTY (Italian with accents)\n", 0x07);
    puts_col("   - ES QWERTY (Spanish with ñ, á, é)\n\n", 0x07);
    
    puts_col("3. USING THE LAYOUT SELECTOR\n", 0x0F);
    puts_col("   Type 'layout' and press ENTER to open the\n", 0x07);
    puts_col("   interactive menu. Use UP/DOWN arrow keys to\n", 0x07);
    puts_col("   select a layout, then press ENTER to confirm.\n\n", 0x07);
    
    puts_col("4. COMMANDS\n", 0x0F);
    puts_col("   - 'help'     : Display all commands\n", 0x07);
    puts_col("   - 'ver'      : Show version info\n", 0x07);
    puts_col("   - 'info'     : System information\n", 0x07);
    puts_col("   - 'clear'    : Clear screen\n", 0x07);
    puts_col("   - 'manual'   : This manual\n", 0x07);
    puts_col("   - 'boot1of'  : Display logo\n\n", 0x07);
    
    puts_col("5. KEYBOARD SHORTCUTS\n", 0x0F);
    puts_col("   - Backspace  : Delete last character\n", 0x07);
    puts_col("   - SHIFT      : Uppercase letters\n", 0x07);
    puts_col("   - ENTER      : Execute command\n\n", 0x07);
    
    puts_col("Press any key to return to prompt...", 0x0E);
    // Wait for key press
    while (get_char_poll() == 0) asm volatile ("pause");
    clear_screen();
    gotoxy(0, 0);
}

static void cmd_boot1of(void) {
    puts_col("\n", 0x07);
    puts_col("+------------------------------+\n", 0x0F);
    puts_col("|                              |\n", 0x0F);
    puts_col("|        HELLO WORLD!          |\n", 0x0F);
    puts_col("|                              |\n", 0x0F);
    puts_col("+------------------------------+\n\n", 0x0F);
}

// ============================================================================
// MAIN KERNEL
// ============================================================================

void kernel_main(void) {
    clear_screen();
    puts_col("Welcome to 1of90OS v", 0x0F);
    putchar_col('0' + VERSION_MAJOR, 0x0F);
    putchar_col('.', 0x0F);
    putchar_col('0' + VERSION_MINOR, 0x0F);
    puts_col("!", 0x0F);
    puts_col("\nType 'help' for commands or 'manual' for user guide.\n\n", 0x07);
    
    // Enable PS/2 keyboard
    outb(0x64, 0xAE);
    for (volatile int i = 0; i < 100000; ++i) asm volatile("");

    char buf[64];
    int idx = 0;
    print_prompt();

    for (;;) {
        unsigned char c = get_char_poll();
        if (!c) {
            asm volatile ("pause");
            continue;
        }

        if (c == '\n') {
            putchar_col('\n', 0x07);
            buf[idx] = '\0';
            
            // Command dispatcher
            if (mystrcmp(buf, "help") == 0) {
                cmd_help();
            } else if (mystrcmp(buf, "ver") == 0 || mystrcmp(buf, "version") == 0) {
                cmd_version();
            } else if (mystrcmp(buf, "info") == 0) {
                cmd_info();
            } else if (mystrcmp(buf, "manual") == 0) {
                cmd_manual();
            } else if (mystrcmp(buf, "clear") == 0) {
                clear_screen();
            } else if (mystrcmp(buf, "boot1of") == 0) {
                cmd_boot1of();
            } else if (mystrcmp(buf, "layout") == 0) {
                layout_menu_loop();
            } else if (idx > 0) {
                puts_col("Unknown command. Type 'help' for list.\n", 0x0C);
            }
            
            idx = 0;
            print_prompt();
            continue;
        }

        if (c == 8) { // backspace
            if (idx > 0) {
                idx--;
                putchar_col('\b', 0x07);
            }
            continue;
        }

        // printable
        if (idx < (int)sizeof(buf)-1) {
            buf[idx++] = c;
            putchar_col(c, 0x07);
        }
    }
}
