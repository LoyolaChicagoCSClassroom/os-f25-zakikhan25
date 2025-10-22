#include "keyboard.h"
#include <stdint.h>

// Low-level I/O functions
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ __volatile__("inb %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "dN"(port));
}

// Map scancodes -> ASCII (US QWERTY layout, simplified)
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',  // 0x00 – 0x07
    '7', '8', '9', '0', '-', '=', '\b',    // 0x08 – 0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y',    // 0x0F – 0x15
    'u', 'i', 'o', 'p', '[', ']', '\n',    // 0x16 – 0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j',  // 0x1D – 0x24
    'k', 'l', ';', '\'', '`', 0, '\\',     // 0x25 – 0x2B
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',// 0x2C – 0x33
    '.', '/', 0, '*', 0, ' ', 0,           // 0x34 – 0x3A
    // Fill rest with 0
};

// External putc from kernel_main
extern int putc(int c);

// Keyboard interrupt handler
void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    // Ignore key release (bit 7 set)
    if (!(scancode & 0x80)) {
        unsigned char ascii = keyboard_map[scancode];
        if (ascii != 0) {
            putc(ascii);
        }
    }

    // Send End of Interrupt (EOI) to PIC
    outb(0x20, 0x20);
}
