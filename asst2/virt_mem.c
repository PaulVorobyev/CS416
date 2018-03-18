#include "data_structure.h"
#include "virt_mem.h"

/* Memory */

Page * mem_init(){
    printf("Creating memory\n");
    page_table = hash_init();

    size_t num_pages = ARRAY_SIZE/PAGE_SIZE;
    int i;
    Page * new_page;

    // Populate the hash table with all empty pages
    for(i = 0; i < num_pages; i++){
        // TODO: create all new pages and set their id = -1, so easily able to find an empty page in the hash table. Need to decide if metadata in page or out of page size
    }
}

/* Pages */

Page * create_new_page(int id, int is_free, size_t req_size){
    // Find a blank page
    hash_find(memory->page_table, -1);

    Page * new_page = (Page *) ( (char *)curr_page + MEM_SIZE + req_size);
}
