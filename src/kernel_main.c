#include <stdint.h>
#include "rprintf.h"
#include "keyboard.h"
#include "io.h"
#include "page.h"
#include "../drivers/fat.h"
#include "../drivers/ide.h"

/*
 * ==============================================
 * HW4: Paging Setup + Keyboard Driver
 * ==============================================
 * Features:
 *  - Basic text output to VGA memory
 *  - Interrupt initialization
 *  - Page Frame Allocator (HW3)
 *  - Paging Setup and Enable (HW4)
 * ==============================================
 */

#define VIDEO_MEMORY 0xB8000
#define ROWS 25
#define COLS 80
#define SCREEN_SIZE (ROWS * COLS)

int cursor = 0;

// --------------------------------------------------
// Screen Output Functions
// --------------------------------------------------
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

// --------------------------------------------------
// Interrupt & PIC Initialization
// --------------------------------------------------
void idt_init(void);
void pic_init(void);

// --------------------------------------------------
// Kernel Entry Point
// --------------------------------------------------
void main(void) {
    clear_screen();
    esp_printf(putc, "CS 310 HW4: Paging Setup\r\n");

    // ---------------------------------------------
    // 1. Initialize the Page Frame Allocator (HW3)
    // ---------------------------------------------
    init_pfa_list();
    esp_printf(putc, "Page Frame Allocator initialized\r\n");

    // ---------------------------------------------
    // 2. Initialize Interrupts (from HW2)
    // ---------------------------------------------
    idt_init();
    pic_init();
    asm("sti");
    esp_printf(putc, "Interrupts enabled\r\n");

    // ---------------------------------------------
    // 3. Setup Paging (HW4)
    // ---------------------------------------------
    extern uint32_t _end_kernel;  // provided by linker

    // Clear out page directory first
    for (int i = 0; i < 1024; i++)
        g_page_directory[i].present = 0;

    // (a) Identity map kernel memory (1MB -> &_end_kernel)
    uint32_t kernel_start = 0x00100000;
    uint32_t kernel_end = (uint32_t)&_end_kernel;
    identity_map_range(kernel_start, kernel_end, g_page_directory);
    esp_printf(putc, "Kernel memory identity-mapped\r\n");

    // (b) Identity map stack region (current ESP - 32KB to ESP)
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    identity_map_range(esp - 0x8000, esp, g_page_directory);
    esp_printf(putc, "Stack memory mapped\r\n");

    // (c) Identity map VGA buffer (0xB8000)
    identity_map_page(0xB8000, g_page_directory);
    esp_printf(putc, "Video memory mapped\r\n");

    // (d) Load Page Directory and enable Paging
    loadPageDirectory(g_page_directory);
    enablePaging();
    esp_printf(putc, "Paging enabled!\r\n");

    // ---------------------------------------------
    // 4. Test output to VGA screen
    // ---------------------------------------------
    volatile uint16_t *vga = (volatile uint16_t*)0xB8000;
    vga[0] = 0x1F00 | 'P';  // 'P' for Paging
    vga[1] = 0x1F00 | 'G';  // 'G' for Good :)

    esp_printf(putc, "PG written to screen (paging test)\r\n");
    esp_printf(putc, "Starting FAT filesystem test...\r\n");

    if (fatInit() == 0) {
        esp_printf(putc, "FAT initialized successfully!\r\n");

        struct file f;
        if (fatOpen("HELLO.TXT", &f) == 0) {
            esp_printf(putc, "File HELLO.TXT found! Size = %d bytes\r\n", f.file_size);

            char buf[128];
            int bytes = fatRead(&f, buf, sizeof(buf) - 1);
            buf[bytes] = '\0';

            esp_printf(putc, "Contents of HELLO.TXT:\r\n%s\r\n", buf);
        } else {
            esp_printf(putc, "HELLO.TXT not found in root directory.\r\n");
        }
    } else {
        esp_printf(putc, "FAT initialization failed.\r\n");
    }

    // ---------------------------------------------
    // 5. Halt the CPU
    // ---------------------------------------------
esp_printf(putc, "System halted. Exiting QEMU...\r\n");

// small delay so output flushes
for (volatile int i = 0; i < 1000000; i++);

asm volatile("cli");   // disable interrupts
asm volatile("hlt");   // halt CPU safely
