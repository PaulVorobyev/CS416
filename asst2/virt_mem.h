#include "data_structure.h"

/* Memory */

typedef struct virtual_memory{
    hash_table page_table;
    char ALLMEM[ARRAY_SIZE];
    Page front;

} mem;

void mem_init();

/* Pages */

typedef struct Page_{
    int id; // id of TCB; -1 for empty/free page
    int is_free; // 1 for free; 0 for malloc'd
    size_t mem_free; // the amount of memory that is free inside this page
    struct Page_ * next;
} Page;


char *expand(MemEntry last, size_t x, char *file, size_t line);
Page * create_new_page(int id, int is_free, size_t req_size);
