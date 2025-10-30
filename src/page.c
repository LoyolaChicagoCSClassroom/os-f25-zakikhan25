#include "page.h"
#include <stddef.h>  // for NULL

/*
 * ==============================================
 * HW3: Page Frame Allocator
 * ==============================================
 * Keeps track of free physical memory pages
 * for later use in paging (HW4)
 * ==============================================
 */

#define PAGE_SIZE (2 * 1024 * 1024)  // 2 MB pages
#define MEMORY_START 0x00000000
#define NUM_PAGES 128

// Array of physical pages in memory
struct ppage physical_page_array[NUM_PAGES];

// Head pointer for free list
static struct ppage *free_physical_pages_head = NULL;

// --------------------------------------------------
// Initialize Page Frame Allocator
// --------------------------------------------------
void init_pfa_list(void) {
    int i;

    physical_page_array[0].next = NULL;
    physical_page_array[0].prev = NULL;
    physical_page_array[0].physical_addr = (void *)MEMORY_START;
    free_physical_pages_head = &physical_page_array[0];

    for (i = 1; i < NUM_PAGES; i++) {
        physical_page_array[i].next = NULL;
        physical_page_array[i].prev = &physical_page_array[i - 1];
        physical_page_array[i].physical_addr = (void *)(MEMORY_START + (i * PAGE_SIZE));
        physical_page_array[i - 1].next = &physical_page_array[i];
    }
}

// --------------------------------------------------
// Allocate npages physical pages
// --------------------------------------------------
struct ppage *allocate_physical_pages(unsigned int npages) {
    struct ppage *allocated_list;
    struct ppage *current;
    unsigned int count;

    if (free_physical_pages_head == NULL || npages == 0) {
        return NULL;
    }

    current = free_physical_pages_head;
    count = 1;

    while (count < npages && current->next != NULL) {
        current = current->next;
        count++;
    }

    if (count < npages) {
        return NULL;
    }

    allocated_list = free_physical_pages_head;
    free_physical_pages_head = current->next;

    if (free_physical_pages_head != NULL) {
        free_physical_pages_head->prev = NULL;
    }

    current->next = NULL;
    return allocated_list;
}

// --------------------------------------------------
// Free a list of physical pages back to the allocator
// --------------------------------------------------
void free_physical_pages(struct ppage *ppage_list) {
    struct ppage *last_page;

    if (ppage_list == NULL) {
        return;
    }

    last_page = ppage_list;
    while (last_page->next != NULL) {
        last_page = last_page->next;
    }

    last_page->next = free_physical_pages_head;

    if (free_physical_pages_head != NULL) {
        free_physical_pages_head->prev = last_page;
    }

    free_physical_pages_head = ppage_list;
    free_physical_pages_head->prev = NULL;
}

// ======================================================
// HW4: Paging Functions
// ======================================================
#include "page.h"
#include <stdint.h>
#include <stddef.h>

// 4KB page tables and directory
struct page_directory_entry g_page_directory[PD_ENTRIES] __attribute__((aligned(4096)));
struct page g_page_tables[PD_ENTRIES][PT_ENTRIES] __attribute__((aligned(4096)));

// ------------------------------------------------------
// Map a single physical page to the same virtual address
// (Identity mapping: VA == PA)
// ------------------------------------------------------
void identity_map_page(uint32_t pa_va, struct page_directory_entry *pd) {
    uint32_t pd_index = PD_INDEX(pa_va);
    uint32_t pt_index = PT_INDEX(pa_va);

    // Create page table if not present
    if (!pd[pd_index].present) {
        pd[pd_index].present = 1;
        pd[pd_index].rw = 1;
        pd[pd_index].user = 0;
        pd[pd_index].frame = ((uint32_t)&g_page_tables[pd_index]) >> 12;
    }

    struct page *pt = g_page_tables[pd_index];
    pt[pt_index].present = 1;
    pt[pt_index].rw = 1;
    pt[pt_index].user = 0;
    pt[pt_index].frame = pa_va >> 12;
}

// ------------------------------------------------------
// Map a range of physical pages with identity mapping
// ------------------------------------------------------
void identity_map_range(uint32_t start_pa_va, uint32_t end_pa_va_exclusive, struct page_directory_entry *pd) {
    uint32_t addr = PAGE_ALIGN_DOWN(start_pa_va);
    while (addr < end_pa_va_exclusive) {
        identity_map_page(addr, pd);
        addr += PAGE_SIZE;
    }
}

// ------------------------------------------------------
// Load Page Directory (writes to CR3)
// ------------------------------------------------------
void loadPageDirectory(struct page_directory_entry *pd) {
    asm volatile("mov %0, %%cr3" :: "r"(pd) : "memory");
}

// ------------------------------------------------------
// Enable Paging (set bits 0 and 31 in CR0)
// ------------------------------------------------------
void enablePaging(void) {
    asm volatile(
        "mov %%cr0, %%eax\n"
        "or $0x80000001, %%eax\n"
        "mov %%eax, %%cr0\n"
        ::: "eax"
    );
}
