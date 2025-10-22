#include "page.h"

#define PAGE_SIZE (2 * 1024 * 1024)
#define MEMORY_START 0x00000000
#define NUM_PAGES 128

struct ppage physical_page_array[NUM_PAGES];
static struct ppage *free_physical_pages_head = 0;

void init_pfa_list(void) {
    int i;
    
    physical_page_array[0].next = 0;
    physical_page_array[0].prev = 0;
    physical_page_array[0].physical_addr = (void *)MEMORY_START;
    free_physical_pages_head = &physical_page_array[0];
    
    for (i = 1; i < NUM_PAGES; i++) {
        physical_page_array[i].next = 0;
        physical_page_array[i].prev = &physical_page_array[i - 1];
        physical_page_array[i].physical_addr = (void *)(MEMORY_START + (i * PAGE_SIZE));
        physical_page_array[i - 1].next = &physical_page_array[i];
    }
}

struct ppage *allocate_physical_pages(unsigned int npages) {
    struct ppage *allocated_list;
    struct ppage *current;
    unsigned int count;
    
    if (free_physical_pages_head == 0 || npages == 0) {
        return 0;
    }
    
    current = free_physical_pages_head;
    count = 1;
    
    while (count < npages && current->next != 0) {
        current = current->next;
        count++;
    }
    
    if (count < npages) {
        return 0;
    }
    
    allocated_list = free_physical_pages_head;
    free_physical_pages_head = current->next;
    
    if (free_physical_pages_head != 0) {
        free_physical_pages_head->prev = 0;
    }
    
    current->next = 0;
    return allocated_list;
}

void free_physical_pages(struct ppage *ppage_list) {
    struct ppage *last_page;
    
    if (ppage_list == 0) {
        return;
    }
    
    last_page = ppage_list;
    while (last_page->next != 0) {
        last_page = last_page->next;
    }
    
    last_page->next = free_physical_pages_head;
    
    if (free_physical_pages_head != 0) {
        free_physical_pages_head->prev = last_page;
    }
    
    free_physical_pages_head = ppage_list;
    free_physical_pages_head->prev = 0;
}
