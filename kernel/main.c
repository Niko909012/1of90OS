#include <stdint.h>

static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    asm volatile ("inb %1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

// --- Serial (COM1) helper routines ---
static inline int serial_is_transmit_empty(void) {
    return inb(0x3F8 + 5) & 0x20;
}

static void serial_init(void) {
    outb(0x3F8 + 1, 0x00); // Disable interrupts
    outb(0x3F8 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x03); // Divisor low byte (38400 baud)
    outb(0x3F8 + 1, 0x00); // Divisor high byte
    outb(0x3F8 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static void serial_putchar(char c) {
    while (!serial_is_transmit_empty()) asm volatile ("pause");
    outb(0x3F8, (unsigned char)c);
}

static inline int serial_data_ready(void) {
    return inb(0x3F8 + 5) & 0x01;
}

static char serial_getchar_nonblock(void) {
    if (!serial_data_ready()) return 0;
    return (char)inb(0x3F8);
}

// Wait until controller input buffer is clear (bit 1 of status = 0)
static void wait_input_clear(void) {
    for (int i = 0; i < 100000; ++i) {
        if ((inb(0x64) & 0x02) == 0) return;
    }
}

// Wait until output buffer is full (bit 0 = 1)
static int wait_output_full(int timeout) {
    for (int i = 0; i < timeout; ++i) {
        if (inb(0x64) & 0x01) return 1;
    }
    return 0;
}

// Send a byte to the keyboard device (via 0xD4 command)
// Send a byte to the primary PS/2 device (keyboard) via data port 0x60
// Note: writing 0xD4 to port 0x64 forwards the next byte to the auxiliary
// device (PS/2 port 2). For the keyboard (primary port) we must write
// directly to 0x60.
static void send_to_keyboard(unsigned char b) {
    wait_input_clear();
    outb(0x60, b);
}

// Try to enable scanning on the keyboard (command 0xF4). Return 1 on ACK, 0 otherwise.
static int enable_keyboard_scanning(void) {
    // Send 0xF4 to device
    send_to_keyboard(0xF4);
    // wait for ACK (0xFA)
    if (!wait_output_full(100000)) return 0;
    unsigned char r = inb(0x60);
    return (r == 0xFA);
}

volatile uint16_t *vga = (volatile uint16_t*)0xB8000;

static int cursor_row = 0;
static int cursor_col = 0;

static void putchar_col(char c, uint8_t color) {
    if (c == '\n') {
        // write newline to serial too
        serial_putchar('\r');
        serial_putchar('\n');
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
    // Mirror to serial for headless testing
    if (c == '\r') {
        /* ignore */
    } else {
        serial_putchar(c);
    }
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

// Two scancode (set 1) maps: US QWERTY and German-like QWERTZ (ASCII approximation)
static const char map_qwerty[128] = {
    0,  0, '1','2','3','4','5','6','7','8','9','0','-','=', 8,  0,
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
    'd','f','g','h','j','k','l',';','\'', '`',  '\\','z','x','c','v',
    'b','n','m',',','.','/',' ', 0,0,0,0,0,0,0,0,0,0,
    /* rest zeros */
};

static const char map_qwertz[128] = {
    0,  0, '1','2','3','4','5','6','7','8','9','0','-','=', 8,  0,
    'q','w','e','r','t','z','u','i','o','p','[',']','\n',0,'a','s',
    'd','f','g','h','j','k','l',';','\'', '`',  '\\','y','x','c','v',
    'b','n','m',',','.','/',' ', 0,0,0,0,0,0,0,0,0,0,
    /* rest zeros */
};

static const char *current_map = map_qwerty;

static char get_char_poll(void) {
    // Poll status port 0x64, bit0 = output buffer full
    if ((inb(0x64) & 1) == 0) return 0;
    unsigned char sc = inb(0x60);
    // Ignore key releases (sc >= 0x80)
    if (sc & 0x80) return 0;
    if (sc < 128) return current_map[sc];
    return 0;
}

static int starts_with(const char *s, const char *prefix) {
    int i = 0;
    while (prefix[i]) {
        if (s[i] != prefix[i]) return 0;
        ++i;
    }
    return 1;
}

static void write_at(int row, int col, const char *s, uint8_t color) {
    int pos = row * 80 + col;
    for (int i = 0; s[i]; ++i) vga[pos + i] = (uint16_t)s[i] | (color << 8);
}

static void set_layout(const char *name) {
    if (starts_with(name, "qwertz") || starts_with(name, "qwert_iso") || starts_with(name, "iso")) {
        current_map = map_qwertz;
        write_at(0, 60, "LAYOUT: QWERTZ   ", 0x0B);
    } else {
        current_map = map_qwerty;
        write_at(0, 60, "LAYOUT: QWERTY   ", 0x0B);
    }
}

/* forward from pci.c */
void pci_scan_and_print(void);

void kernel_main(void) {
    // clear screen
    // initialize serial early so messages appear in `-serial stdio`
    serial_init();

    for (int i = 0; i < 80*25; ++i) vga[i] = (uint16_t)(' ') | (0x07 << 8);

    puts_col("Welcome to 1of90OS!\n", 0x07);
    // Try to enable PS/2 port 1 (keyboard) via controller command 0xAE
    outb(0x64, 0xAE);
    // small pause
    for (volatile int i = 0; i < 100000; ++i) asm volatile("");

    // show PS/2 controller status at top-right
    puts_col("PS2 STATUS: ", 0x0E);
    // read status once and print hex
    unsigned char st = inb(0x64);
    char hex[3];
    const char *hexd = "0123456789ABCDEF";
    hex[0] = hexd[(st >> 4) & 0xF];
    hex[1] = hexd[st & 0xF];
    hex[2] = '\0';
    puts_col(hex, 0x0E);
    puts_col("\n", 0x07);
    cursor_row = 1; cursor_col = 0;

    // Try to enable keyboard scanning and show result
    int kb_ok = enable_keyboard_scanning();
    if (kb_ok) write_at(0, 50, "KB SCAN: OK ", 0x0A);
    else write_at(0, 50, "KB SCAN: NOACK", 0x0C);

    /* scan PCI for USB controllers and print results */
    pci_scan_and_print();

    const char *prompt = "1of90OS> ";
    puts_col(prompt, 0x07);

    char buf[64];
    int idx = 0;

    for (;;) {
        // show live status in column 72 of row 0
        unsigned char status_now = inb(0x64);
        int colpos = 72;
        int pos = 0*80 + colpos;
        // print two hex digits of status
        vga[pos+0] = (uint16_t)hexd[(status_now >> 4) & 0xF] | (0x0E << 8);
        vga[pos+1] = (uint16_t)hexd[status_now & 0xF] | (0x0E << 8);

        char c = get_char_poll();
        if (!c) {
            /* if no PS/2 char available, try serial (non-blocking). This
               makes the kernel usable when QEMU is started with
               `-serial stdio` so typed chars arrive via COM1. */
            c = serial_getchar_nonblock();
        }

        if (!c) {
            // idle briefly to reduce busy waiting; avoid HLT because interrupts are disabled
            asm volatile ("pause");
            continue;
        }

        if (c == '\n' || c == '\r') {
            putchar_col('\n', 0x07);
            buf[idx] = '\0';
            // handle command
            if (mystrcmp(buf, "boot1of") == 0) {
                // print ASCII art for HELLO WORLD !
                puts_col(" _   _      _ _        _ _ \\n+", 0x0F);
                puts_col("| | | | ___| | | ___  | | |\n", 0x0F);
                puts_col("| |_| |/ _ \\ | |/ _ \\ | | |\n", 0x0F);
                puts_col("|  _  |  __/ | | (_) || |_|\n", 0x0F);
                puts_col("|_| |_|\\___|_|_|\\___/ (_) \n", 0x0F);
                puts_col("                          !\n", 0x0F);
            } else if (starts_with(buf, "layout")) {
                // parse layout argument: "layout qwertz" or "layout qwerty"
                int i = 6;
                while (buf[i] == ' ') ++i;
                if (buf[i]) set_layout(&buf[i]);
                else set_layout("qwerty");
            } else if (idx > 0) {
                puts_col("Unknown command\n", 0x07);
            }
            // new prompt
            idx = 0;
            puts_col(prompt, 0x07);
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
            // also print scancode hex to diagnostic area (row 2, cols 72-75)
            unsigned char last_sc = inb(0x60);
            int diag_row = 2;
            int diag_col = 72;
            int p = diag_row * 80 + diag_col;
            vga[p+0] = (uint16_t)hexd[(last_sc >> 4) & 0xF] | (0x0C << 8);
            vga[p+1] = (uint16_t)hexd[last_sc & 0xF] | (0x0C << 8);
            // show ascii if printable
            char outch = (c >= 32 && c < 127) ? c : '?';
            vga[p+3] = (uint16_t)outch | (0x0C << 8);
        }
    }
}
