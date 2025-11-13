#include <stdint.h>
#include "../src/rprintf.h"
#include "ide.h"

// Local print helper (renamed to avoid collision with kernel's putc)
int ide_putc(int c) {
    volatile uint16_t *vga = (volatile uint16_t*)0xB8000;
    static int pos = 0;
    vga[pos++] = (0x07 << 8) | c;
    if (pos >= 80 * 25) pos = 0;
    return c;
}

// Simulated ATA read for testing FAT driver
int ata_lba_read(uint32_t lba, unsigned char *buf, uint32_t nsectors) {
    esp_printf(ide_putc, "ata_lba_read called: LBA=%d, sectors=%d\r\n", lba, nsectors);
    return 0;
}
