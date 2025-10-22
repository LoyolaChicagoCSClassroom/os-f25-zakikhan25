#include "interrupt.h"
#include <stdint.h>
#include "io.h"

// The IDT with 256 entries
static struct idt_entry idt[256];
static struct idt_ptr idtp;

// Load the IDT (from inline assembly)
extern void idt_load(void);

// Simple ISR handler for keyboard (scancode in port 0x60)
extern void keyboard_handler(void);

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt[i].base_lo = 0;
        idt[i].base_hi = 0;
        idt[i].sel = 0;
        idt[i].always0 = 0;
        idt[i].flags = 0;
    }

    // Install keyboard handler on interrupt 33 (IRQ1)
    idt_set_gate(33, (uint32_t)keyboard_handler, 0x08, 0x8E);

    // Load IDT
    idt_load();
}

// Stub for PIC initialization
void pic_init(void) {
    // Remap PIC (real code would send ICWs to ports 0x20/0xA0)
    // For now, just unmask keyboard IRQ (IRQ1).
    outb(0x21, 0xFD);  // 11111101 -> enable only IRQ1 (keyboard)
    outb(0xA1, 0xFF);  // disable all IRQs on slave PIC
}

// Stub for loading IDT
void idt_load(void) {
    // This normally uses lidt assembly instruction with IDT pointer
    // For now, placeholder so linker doesn’t fail
    __asm__ __volatile__("nop");
}
