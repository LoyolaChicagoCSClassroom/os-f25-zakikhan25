#include <stdint.h>
#include <string.h>
#include "fat.h"
#include "ide.h"   // for ata_lba_read()
#include "../src/rprintf.h"

#define SECTOR_SIZE 512

static struct boot_sector bs;
static unsigned char fat_table[8192];
static unsigned int root_sector;

static int sd_readblock(uint32_t lba, void *buf, uint32_t nsectors) {
    return ata_lba_read(lba, (unsigned char*)buf, nsectors);
}

int fatInit() {
    unsigned char buf[SECTOR_SIZE];
    if (sd_readblock(0, buf, 1) != 0) return -1;

    for (int i = 0; i < SECTOR_SIZE; i++)
        ((unsigned char*)&bs)[i] = buf[i];

    if (bs.boot_signature != 0xAA55) return -1;

    root_sector = bs.num_reserved_sectors +
                  (bs.num_fat_tables * bs.num_sectors_per_fat);

    sd_readblock(bs.num_reserved_sectors, fat_table, bs.num_sectors_per_fat);
    return 0;
}

// Simple kernel-level strcmp (since libc is unavailable)
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


int fatOpen(const char *filename, struct file *out) {
    unsigned char buf[SECTOR_SIZE * 32];
    unsigned int sectors = 32;

    sd_readblock(root_sector, buf, sectors);
    struct root_directory_entry *rde = (struct root_directory_entry*)buf;

    for (int i = 0; i < bs.num_root_dir_entries; i++) {
        if (rde[i].file_name[0] == 0x00) break;
        if ((rde[i].attribute & 0x0F) == 0x0F) continue;

        char name[13];
        int k = 0;
        while (k < 8 && rde[i].file_name[k] != ' ')
            name[k] = rde[i].file_name[k], k++;
        name[k] = '\0';
        if (rde[i].file_extension[0] != ' ') {
            name[k++] = '.';
            int n = 0;
            while (n < 3 && rde[i].file_extension[n] != ' ')
                name[k++] = rde[i].file_extension[n++];
            name[k] = '\0';
        }

        if (strcmp(name, filename) == 0) {
            *out = (struct file){ .rde = rde[i],
                                  .start_cluster = rde[i].cluster,
                                  .file_size = rde[i].file_size };
            return 0;
        }
    }
    return -1;
}

int fatRead(struct file *f, void *buf, uint32_t nbytes) {
    unsigned int cluster = f->start_cluster;
    unsigned int bytes_per_cluster = bs.bytes_per_sector * bs.num_sectors_per_cluster;
    unsigned int remaining = (nbytes > f->file_size) ? f->file_size : nbytes;
    unsigned char cluster_buf[SECTOR_SIZE];
    unsigned char *dst = buf;

    while (remaining > 0 && cluster >= 2 && cluster < 0xFF8) {
        unsigned int lba = root_sector + bs.num_root_dir_entries / 16 +
                           (cluster - 2) * bs.num_sectors_per_cluster;
        sd_readblock(lba, cluster_buf, 1);
        unsigned int chunk = (remaining < bytes_per_cluster) ? remaining : bytes_per_cluster;
        for (unsigned i = 0; i < chunk; i++) dst[i] = cluster_buf[i];
        dst += chunk;
        remaining -= chunk;
        cluster++;
    }
    return nbytes - remaining;
}
