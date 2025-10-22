#include <stdint.h>
#include "rprintf.h"
#include "keyboard.h"
#include "io.h"
#include "page.h"

#define VIDEO_MEMORY 0xB8000
#define ROWS 25
#define COLS 80
#define SCREEN_SIZE (ROWS * COLS)

int cursor = 0;

// Screen output functions
void scroll_screen(void) {
    char *video = (char*) VIDEO_MEMORY;
    for (int i = 0; i < (ROWS - 1) * COLS * 2; i++) {
        video[i] = video[i + COLS * 2];
    }
    for (int i = (ROWS - 1) * COLS * 2; i < ROWS * COLS * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x07;
    }
    cursor = (ROWS - 1) * COLS;
}

int putc(int data) {
    char *video = (char*) VIDEO_MEMORY;
    if (data == '\n') {
        cursor += COLS - (cursor % COLS);
    } else {
        video[cursor * 2] = (char) data;
        video[cursor * 2 + 1] = 0x07;
        cursor++;
    }
    if (cursor >= SCREEN_SIZE) scroll_screen();
    return data;
}

void clear_screen(void) {
    char *video = (char*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_SIZE * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x07;
    }
    cursor = 0;
}




// === STUBS (you must have real ones in interrupt.c / idt.c) ===
void idt_init(void);
void pic_init(void);

// Kernel entry point
void main(void) {
    clear_screen();
    esp_printf(putc, "CS 310 HW2: Keyboard Driver (Interrupts)\r\n");

    // Initialize the page frame allocator
    init_pfa_list();                                          // <--- ADD THIS
    esp_printf(putc, "Page allocator initialized\r\n");       // <--- ADD THIS


    // Initialize interrupts
    idt_init();
    pic_init();
    asm("sti");  // Enable interrupts

    // Wait forever for keyboard input
    while (1) {
        asm("hlt");
    }
}

