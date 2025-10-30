#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

/*
 * ============================================
 * HW4: Paging + Page Frame Allocator Header
 * ============================================
 * This file combines:
 *  - Physical Page Allocator (from HW3)
 *  - Paging data structures and functions (for HW4)
 */

// ---------- Page Frame Allocator (from HW3) ----------

struct ppage {
    struct ppage *next;   // next page in list
    struct ppage *prev;   // previous page in list
    void *physical_addr;  // physical address of this page
};

void init_pfa_list(void);
struct ppage *allocate_physical_pages(unsigned int npages);
void free_physical_pages(struct ppage *ppage_list);

// ---------- Paging Structures (HW4) ----------

// Page Directory Entry (1024 entries total)
struct page_directory_entry {
    uint32_t present       : 1;   // 1 = page present in memory
    uint32_t rw            : 1;   // 1 = read/write, 0 = read-only
    uint32_t user          : 1;   // 1 = user mode, 0 = kernel only
    uint32_t writethru     : 1;   // cache write-through
    uint32_t cachedisabled : 1;   // disable caching
    uint32_t accessed      : 1;   // page accessed flag
    uint32_t pagesize      : 1;   // 0 = 4KB page, 1 = 4MB page
    uint32_t ignored       : 2;   // ignored/reserved bits
    uint32_t os_specific   : 3;   // OS can use these bits
    uint32_t frame         : 20;  // physical address >> 12
};

// Page Table Entry (1024 entries per table)
struct page {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t accessed : 1;
    uint32_t dirty    : 1;
    uint32_t unused   : 7;
    uint32_t frame    : 20;   // physical address >> 12
};

// ---------- Constants ----------

#define PAGE_SIZE 4096
#define PD_ENTRIES 1024
#define PT_ENTRIES 1024

// Helper macros to find indexes
#define PD_INDEX(vaddr) (((uint32_t)(vaddr) >> 22) & 0x3FF)
#define PT_INDEX(vaddr) (((uint32_t)(vaddr) >> 12) & 0x3FF)
#define PAGE_ALIGN_DOWN(x) ((uint32_t)(x) & ~(PAGE_SIZE - 1))

// ---------- Global Page Tables ----------

extern struct page_directory_entry g_page_directory[PD_ENTRIES];
extern struct page g_page_tables[PD_ENTRIES][PT_ENTRIES];

// ---------- Function Prototypes ----------

// Map list of physical pages to a virtual address
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);

// Identity map a single or range of pages (same VA and PA)
void identity_map_page(uint32_t pa_va, struct page_directory_entry *pd);
void identity_map_range(uint32_t start_pa_va, uint32_t end_pa_va_exclusive, struct page_directory_entry *pd);

// Load page directory and enable paging (inline assembly)
void loadPageDirectory(struct page_directory_entry *pd);
void enablePaging(void);

#endif // PAGE_H

