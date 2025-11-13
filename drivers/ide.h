#ifndef IDE_H
#define IDE_H

#include <stdint.h>

// Basic sector read stub used by FAT driver
int ide_read_sector(uint32_t lba, void *buf);

#endif
