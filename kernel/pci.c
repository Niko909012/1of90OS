#include <stdint.h>

/* Simple PCI config space access via I/O ports 0xCF8/0xCFC */
static inline void outl(unsigned short port, unsigned int val) {
    asm volatile ("outl %0, %1" : : "a" (val), "Nd" (port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int val;
    asm volatile ("inl %1, %0" : "=a" (val) : "Nd" (port));
    return val;
}

/* Build config address */
static unsigned int pci_cfg_addr(unsigned int bus, unsigned int dev, unsigned int func, unsigned int offset) {
    return (unsigned int)(0x80000000U | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC));
}

static unsigned int pci_read_cfg_dword(unsigned int bus, unsigned int dev, unsigned int func, unsigned int offset) {
    outl(0xCF8, pci_cfg_addr(bus, dev, func, offset));
    return inl(0xCFC);
}

/* VGA output used for simple diagnostics (assumes `vga` is defined in main.c) */
extern volatile uint16_t *vga;

static void write_at(int row, int col, const char *s, uint8_t color) {
    int pos = row * 80 + col;
    for (int i = 0; s[i]; ++i) {
        vga[pos + i] = (uint16_t)s[i] | (color << 8);
    }
}

void pci_scan_and_print(void) {
    int out_row = 4;
    write_at(out_row++, 0, "Scanning PCI for USB controllers...", 0x0E);

    int found = 0;
    for (unsigned int bus = 0; bus < 1; ++bus) {
        for (unsigned int dev = 0; dev < 32; ++dev) {
            for (unsigned int func = 0; func < 8; ++func) {
                unsigned int d = pci_read_cfg_dword(bus, dev, func, 0x00);
                unsigned short vendor = d & 0xFFFF;
                if (vendor == 0xFFFF) continue;
                unsigned short device = (d >> 16) & 0xFFFF;

                unsigned int cls = pci_read_cfg_dword(bus, dev, func, 0x08);
                unsigned char base_class = (cls >> 24) & 0xFF;
                unsigned char sub_class = (cls >> 16) & 0xFF;
                unsigned char prog_if = (cls >> 8) & 0xFF;

                if (base_class == 0x0C && sub_class == 0x03) {
                    char buf[80];
                    int n = 0;
                    // simple hex formatting
                    const char *hex = "0123456789ABCDEF";
                    // vendor/device
                    buf[n++] = 'U'; buf[n++] = 'S'; buf[n++] = 'B'; buf[n++] = ':'; buf[n++] = ' ';
                    // bus:dev.func
                    buf[n++] = '0' + (char)bus; buf[n++] = ':';
                    if (dev >= 10) { buf[n++] = '0' + (dev/10); buf[n++] = '0' + (dev%10);} else { buf[n++] = '0' + dev; }
                    buf[n++] = '.'; buf[n++] = '0' + func; buf[n++] = ' ';
                    buf[n++] = 'V'; buf[n++] = '=';
                    // vendor hex (4 digits)
                    buf[n++] = hex[(vendor >> 12) & 0xF]; buf[n++] = hex[(vendor >> 8) & 0xF];
                    buf[n++] = hex[(vendor >> 4) & 0xF]; buf[n++] = hex[vendor & 0xF]; buf[n++] = ' ';
                    buf[n++] = 'D'; buf[n++] = '=';
                    buf[n++] = hex[(device >> 12) & 0xF]; buf[n++] = hex[(device >> 8) & 0xF];
                    buf[n++] = hex[(device >> 4) & 0xF]; buf[n++] = hex[device & 0xF]; buf[n++] = 0;
                    write_at(out_row++, 0, buf, 0x0A);
                    found++;
                }
            }
        }
    }

    if (found == 0) write_at(out_row++, 0, "No USB controllers found.", 0x0C);
}
