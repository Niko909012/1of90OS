#include <stdint.h>

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

static void putchar_col(char c, uint8_t color) {
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
            for (int c = 0; c < 80; ++c) {
                vga[(r-1)*80 + c] = vga[r*80 + c];
            }
        }
        // clear last line
        for (int c = 0; c < 80; ++c) vga[24*80 + c] = (uint16_t)(' ') | (0x07 << 8);
        cursor_row = 24;
    }
    int idx = cursor_row * 80 + cursor_col;
    vga[idx] = (uint16_t)c | (color << 8);
    cursor_col++;
}

static void puts_col(const char *s, uint8_t color) {
    for (int i = 0; s[i]; ++i) putchar_col(s[i], color);
}

// Simple strcmp (no libc)
static int mystrcmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return (int)(a[i] - b[i]);
        ++i;
    }
    return (int)(a[i] - b[i]);
}

// Layout selection (initial values)
typedef enum { LAYOUT_US = 0, LAYOUT_DE = 1 } layout_t;
static layout_t current_layout = LAYOUT_US;
static const char *layout_name = "us";

// Shift state tracking (scancodes 0x2A, 0x36 pressed; releases 0xAA, 0xB6)
static int shift_pressed = 0;

// Prompt prefix and helper (file scope)
static const char *prompt_prefix = "1of90OS[";
static void print_prompt(void) {
    cursor_col = 0;
    puts_col(prompt_prefix, 0x07);
    puts_col(layout_name, 0x0E);
    puts_col("]> ", 0x07);
}

// Two simple scancode maps (set 1): US QWERTY and DE QWERTZ (lowercase)
// Use explicit PS/2 set1 scancode indices for clarity
static const char scancode_us[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = 8, /* backspace */
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

// German QWERTZ mapping: swap y/z positions and keep punctuation similar
static const char scancode_de[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = 8,
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'z', /* y<->z */ [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'y', /* swapped */ [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '-', [0x39] = ' '
};



static int starts_with(const char *s, const char *pre) {
    int i = 0;
    while (pre[i]) {
        if (s[i] != pre[i]) return 0;
        i++;
    }
    return 1;
}

static void set_layout_by_name(const char *name) {
    if (name == 0) return;
    if (mystrcmp(name, "us") == 0) {
        current_layout = LAYOUT_US;
        layout_name = "us";
    } else if (mystrcmp(name, "de") == 0) {
        current_layout = LAYOUT_DE;
        layout_name = "de";
    }
}

static char map_scancode(unsigned char sc) {
    const char *map = (current_layout == LAYOUT_US) ? scancode_us : scancode_de;
    if (sc >= 128) return 0;
    char base = map[sc];
    if (!base) return 0;
    // apply simple shift: letters -> uppercase
    if (shift_pressed && base >= 'a' && base <= 'z') return (char)(base - 'a' + 'A');
    return base;
}

static char get_char_poll(void) {
    // Poll status port 0x64, bit0 = output buffer full
    if ((inb(0x64) & 1) == 0) return 0;
    unsigned char sc = inb(0x60);
    unsigned char code = sc & 0x7F;
    int released = sc & 0x80;

    // Shift keys handling
    if (code == 0x2A || code == 0x36) {
        if (released) shift_pressed = 0; else shift_pressed = 1;
        return 0;
    }

    // Ignore releases for other keys
    if (released) return 0;

    return map_scancode(code);
}

void kernel_main(void) {
    // clear screen
    for (int i = 0; i < 80*25; ++i) vga[i] = (uint16_t)(' ') | (0x07 << 8);

    puts_col("Welcome to 1of90OS!\n", 0x07);
    // Try to enable PS/2 port 1 (keyboard) via controller command 0xAE
    outb(0x64, 0xAE);
    // small pause
    for (volatile int i = 0; i < 100000; ++i) asm volatile("");

    cursor_row = 1; cursor_col = 0;

    // initial prompt will be printed below

    char buf[64];
    int idx = 0;

    for (;;) {
        /* live PS/2 status display removed */

        char c = get_char_poll();
        if (!c) {
            // idle briefly to reduce busy waiting; avoid HLT because interrupts are disabled
            asm volatile ("pause");
            continue;
        }

        if (c == '\n') {
            putchar_col('\n', 0x07);
            buf[idx] = '\0';
            // handle command
            if (mystrcmp(buf, "boot1of") == 0) {
                // Simple, robust boxed HELLO WORLD message
                puts_col("+------------------------------+\n", 0x0F);
                puts_col("|                              |\n", 0x0F);
                puts_col("|        HELLO WORLD!          |\n", 0x0F);
                puts_col("|                              |\n", 0x0F);
                puts_col("+------------------------------+\n\n", 0x0F);
            } else if (starts_with(buf, "layout ")) {
                // layout command: "layout us" or "layout de"
                const char *arg = buf + 7;
                set_layout_by_name(arg);
                puts_col("Layout set to ", 0x07);
                puts_col(layout_name, 0x0E);
                puts_col("\n", 0x07);
            } else if (idx > 0) {
                puts_col("Unknown command\n", 0x07);
            }
            // new prompt
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
            /* diagnostic scancode display removed */
        }
    }
}
