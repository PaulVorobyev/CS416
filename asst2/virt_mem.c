#include "virt_mem.h"

void mem_init(char allmem[]){
    void * end_of_mdata = create_mdata(allmem);
    create_pagetable(allmem, end_of_mdata);

    // claim first page for sys
    Page *first_page = (Page*) allmem;
    *first_page = (Page) {
        .id = 0,
        .is_free = 0,
        .idx = 0,
        .mem_free = PAGE_SIZE,
        .next = first_page->next,
        .prev = first_page->prev,
        .front = (Entry*) allmem };
}

void *create_pagetable(char allmem[], void * end_of_mdata){
    printf("Creating PT and SysInfo \n");
    int prev_remaining_space = ((Entry*)(((char*)end_of_mdata) - sizeof(Entry)))->size;

    // SysInfo
    Entry *sys_info_entry = ((Entry*)end_of_mdata) - 1;
    *sys_info_entry = (Entry) {
        .size = sizeof(SysInfo),
        .is_free = 0,
        .next = (Entry*) (((char*)(sys_info_entry + 1)) + sizeof(SysInfo)) };
    SysInfo *info = (SysInfo*) (sys_info_entry + 1);
    *info = (SysInfo) {
        .pagetable = (PTE**) (sys_info_entry->next + 1),
        .mdata = (Page*) (allmem + sizeof(Entry)) };

    // page_table outer
    Entry *page_table_outer_entry = sys_info_entry->next;
    *page_table_outer_entry = (Entry) {
        .size = sizeof(PTE*),
        .is_free = 0,
        .next = (Entry*) (((char*)(page_table_outer_entry + 1)) + sizeof(PTE*)) };
    PTE **page_table_outer = (PTE**) (page_table_outer_entry + 1);
    *page_table_outer = (PTE*) (((char*)(page_table_outer + 1)) + sizeof(Entry));

    // page_table inner
    Entry *page_table_inner_entry = page_table_outer_entry->next;
    *page_table_inner_entry = (Entry) {
        .size = sizeof(PTE),
        .is_free = 0,
        .next = (Entry*) (((char*)(page_table_inner_entry + 1)) + sizeof(PTE)) };
    page_table_outer[0][0] = (PTE) {
        .page_index = 0,
        .page_loc = (void*) allmem,
        .next = (void*) NULL };

    // remaining space entry
    Entry *remaining_entry = page_table_inner_entry->next;
    int size_of_everything_we_made = sizeof(SysInfo) + sizeof(Entry) + sizeof(PTE*) + sizeof(Entry) + sizeof(PTE) + sizeof(Entry);
    *remaining_entry = (Entry) {
        .size = prev_remaining_space - size_of_everything_we_made,
        .is_free = 1,
        .next = NULL };

    return (void*)(remaining_entry + 1);
}

void *create_mdata(char allmem[]){
    printf("Creating page meta data \n");
    int num_pages = (int)(ARRAY_SIZE/PAGE_SIZE);
    int mdata_size = sizeof(Page) * num_pages;

    // make an Entry for mdata
    Entry *mdata_entry = (Entry*) allmem;
    *mdata_entry = (Entry) {
        .size = mdata_size,
        .next = (Entry*) allmem + sizeof(Entry) + mdata_size,
        .is_free = 0 };

    int i;
    Page * root = (Page*) (allmem + sizeof(Entry));
    Page * curr_page = root;
    Page * prev_page = NULL;

    // Populate the page metadata (in OS land) with all empty pages
    for(i = 0; i < num_pages; i++){
        *curr_page = (Page) {
            .id = -1,
            .is_free = 1,
            .mem_free = PAGE_SIZE,
            .next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE),
            .prev = prev_page,
            .front = NULL,
            .idx = -1 };
        
        prev_page = curr_page;
        curr_page = curr_page->next;
    }
    curr_page->next = NULL;

    // make an Entry for remaining space in Page
    Entry *remaining_entry = mdata_entry->next;
    *remaining_entry = (Entry) {
        .size = PAGE_SIZE - (sizeof(Entry) + mdata_entry->size + sizeof(Entry)),
        .next = NULL,
        .is_free = 1 };

    return (void *)(remaining_entry + 1);
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

void user_malloc(char ** allmem, int ** page_table, Page * last_page,  int req_pages){
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

void* find_page(int id, int size) {

}

