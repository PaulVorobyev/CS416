#include <stddef.h>
#include <unistd.h>

// Size of total memory array
#define ARRAY_SIZE 8388608 //TODO: should this not be 8 miliion flat?
// Size of Page struct
#define PAGE_STRUCT_SIZE sizeof(struct Page_)
// Size of the system page itself
#define PAGE_SIZE (sysconf(_SC_PAGE_SIZE))

void mem_init(char allmem[]);
void create_pagetable(char ** allmem, void * end_of_mdata);
void * create_mdata(char ** allmem);
void print_mem(char ** allmem);
int ceil (double num);

/* Pages */

typedef struct Page_{
    int id; // id of TCB; -1 for empty/free page
    //TODO: can later change to check if mem_free == however much a freepage is
    int is_free; // 1 for free; 0 for malloc'd
    size_t mem_free; // the amount of memory that is free inside this page TODO: do we need?
    struct Page_ * next; //TODO: do we need?
    struct Page_ * prev; //TODO: do we need?
    struct Entry_ * front; //TODO: do we need?
    int idx;
} Page;

// Sub-data structure within a page

typedef struct Entry_{
    size_t size;
    struct Entry_ * next;
    int is_free;
} Entry;

typedef struct Page_Table_Entry_{
    // page_table[x] == all of the Page_Table_Entry's with tcb_id x
    int page_index; // virtual address
    void * page_loc; // physical address
    struct Page_Table_Entry_ * next; //TODO: do we need?
} PTE;

typedef struct SysInfo_ {
    PTE* *pagetable;
    Page *mdata;
} SysInfo;

Page * create_new_page(int id, int is_free, size_t req_size);
