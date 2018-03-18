#include <unistd.h>
#include "virt_mem.h"

/* Memory */

void mem_init(char ** ALLMEM){
    printf("Creating memory\n");

    printf("%d\n", sysconf( _SC_PAGE_SIZE));
    int i;
    Page * root = (Page *)(ALLMEM);
    Page * curr_page = root;
    Page * prev_page = NULL;

    // Populate the hash table with all empty pages
    for(i = 1; i < (int)(ARRAY_SIZE/PAGE_SIZE); i++){
        printf("i: %d\n", i);
        // Create page- Assuming that metadata is part of the page size
        curr_page->id = -1;
        curr_page->is_free = 1;
        curr_page->mem_free = PAGE_SIZE - PAGE_STRUCT_SIZE;
        curr_page->next = (Page *) ( (char *)curr_page + PAGE_SIZE);
        curr_page->prev = prev_page;
        curr_page->front = NULL;
        
        prev_page = curr_page;
        curr_page = curr_page->next;
    }
    curr_page->next = NULL;
    print_mem(ALLMEM);
}

void print_mem(char ** ALLMEM){
    printf("Printing all pages\n");
    Page * root = (Page *)ALLMEM;
    Page * curr_page;
    int i = 0;

    for (curr_page = root; curr_page; curr_page=curr_page->next, i++){
        printf("Page %d: %d\n", i, (int)curr_page);
    }
}

/* Pages */

Page * create_new_page(int id, int is_free, size_t req_size){
    // Find a blank page
}
