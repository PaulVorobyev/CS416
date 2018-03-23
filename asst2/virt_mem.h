#include <stddef.h>
#include <unistd.h>

#ifndef VIRT_MEM_H
#define VIRT_MEM_H

// Size of total memory array
#define ARRAY_SIZE (8388608)
// Size of Page struct
#define PAGE_STRUCT_SIZE (sizeof(struct Page_))
// Size of the system page itself
#define PAGE_SIZE (sysconf(_SC_PAGE_SIZE))
// total number of pages in memory
#define NUM_PAGES ((int)(ARRAY_SIZE/PAGE_SIZE))
// pointer to SysInfo
#define SYSINFO ((SysInfo*) (allmem + (PAGE_SIZE * my_ceil((double)(sizeof(Page) * NUM_PAGES) / (double)PAGE_SIZE)) + sizeof(Entry)))
// pointer to pagetable
#define PAGETABLE (SYSINFO->pagetable)
// pointer to mdata
#define MDATA (SYSINFO->mdata)
// number of pages allocated for thread with id x
#define GET_NUM_PTES(x) (((Entry*)(((char*) (&PAGETABLE[x][0])) - sizeof(Entry)))->size / sizeof(PTE))

char *allmem;

void mem_init();
void *create_pagetable(void * end_of_mdata);
void *create_mdata();
void print_mem();
int my_ceil(double num);
void *single_page_malloc(int size, int id);
void *multi_page_malloc(int req_pages, int size, int id);

/* Pages */

typedef struct Page_{
    int id; // id of TCB; -1 for empty/free page
    //TODO: can later change to check if mem_free == however much a freepage is
    int is_free; // 1 for free; 0 for malloc'd
    size_t mem_free; // the amount of memory that is free inside this page TODO: do we need?
    struct Page_ * next; //TODO: do we need?
    struct Page_ * prev; //TODO: do we need?
    struct Entry_ * front;
    int idx;
    int cur_idx;
    int parent; // for multipage requests, idx of first page
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


#endif
