#include "virt_mem.h"

/* Memory */

void mem_init(char ** allmem){
    void * end_of_mdata = create_mdata(allmem);
    create_pagetable(allmem, end_of_mdata);
    
    return ;
}

PTE ** create_pagetable(char ** allmem, void * end_of_mdata){
    // assume at this point, we only have 1 "thread" which is SYS
    // anything that's SYS, would belong to the first row/index
    PTE **page_table = (PTE**) end_of_mdata;
    PTE * pte_ptr = page_table + 1;
    *pte_ptr = (PTE) {
        .page_index = 0,
        .page_loc = 0,
        .next = 0 };
    PTE * front = (PTE *) (end_of_mdata);
    PTE * page_table[1];
    page_table[1] = 
    
}

void * create_mdata(char ** allmem){
    printf("Creating page meta data \n");

    printf("Page size: %d\n", sysconf( _SC_PAGE_SIZE));
    int i;
    Page * root = (Page *)(allmem);
    Page * curr_page = root;
    Page * prev_page = NULL;

    // Populate the page metadata (in OS land) with all empty pages
    for(i = 0; i < (int)(ARRAY_SIZE/PAGE_SIZE); i++){
        printf("i: %d\n", i);
        // Create page- Assuming that metadata is part of the page size
        curr_page->id = -1;
        curr_page->is_free = 1;
        curr_page->mem_free = PAGE_SIZE;
        curr_page->next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE);
        curr_page->prev = prev_page;
        curr_page->front = NULL;
        
        prev_page = curr_page;
        curr_page = curr_page->next;
    }
    curr_page->next = NULL;
    print_mem(allmem);
    return (void *)(curr_page + PAGE_STRUCT_SIZE);
}

void print_mem(char ** allmem){
    printf("Printing all pages\n");
    Page * root = (Page *)allmem;
    Page * curr_page;
    int i = 0;

    for (curr_page = root; curr_page; curr_page=curr_page->next, i++){
        printf("Page %d: %d\n", i, (int)curr_page);
    }
    printf("# of pages: %d\n", (ARRAY_SIZE/PAGE_SIZE));
}

void sys_malloc(char ** allmem, int ** page_table, Page * last_page,  int req_pages){
    printf("System malloc\n");
    // go back req_pages pages (ex: req_pages = 4, go to 4th to last page)
    printf("last: %d\n", (int)(last_page));
    //TODO: FIX LAST
    int i = req_pages;
    //for(; i >= 0; i--){
        
    //}
}

int ceil(double num){
    if (num == (int)num) {
        return (int)num;
    }
    return (int)num + 1;
}

/* Pages */

Page * create_new_page(int id, int is_free, size_t req_size){
    
}
