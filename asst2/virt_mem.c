#include "data_structure.h"

Page * mem_init(){
    printf("Creating memory\n");
    page_table = hash_init();
    return create_new_page(-1, 0, ARRAY_SIZE-MEM_SIZE);
}

Page * create_new_page(int id, int is_free){
    Page * new_page = 
}
