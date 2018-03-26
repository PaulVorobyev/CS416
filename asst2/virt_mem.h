#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

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
// max size of a mementry
#define MAX_ENTRY_SIZE (PAGE_SIZE - sizeof(Entry))
// SIZE OF MDATA IN PAGE NUMBERS //28-ish
#define MDATA_NUM_PAGES (my_ceil((double)(sizeof(Page) * NUM_PAGES) / (double)PAGE_SIZE))
// size of mdata
#define MDATA_SIZE (MDATA_NUM_PAGES*PAGE_SIZE)
// proper address for a Page in allmem, given its idx
#define GET_PAGE_ADDRESS(x) ((void *) (allmem + (PAGE_SIZE * x)))
// pointer to SysInfo
#define SYSINFO ((SysInfo*) (allmem + ((NUM_PAGES - MDATA_NUM_PAGES - 1) * PAGE_SIZE) + sizeof(Entry)))
// pointer to pagetable
#define PAGETABLE (SYSINFO->pagetable)
// pointer to mdata
#define MDATA (SYSINFO->mdata)
// number of pages allocated for thread with id x
#define GET_NUM_PTES(x) (((Entry*)(((char*) (&PAGETABLE[x][0])) - sizeof(Entry)))->size / sizeof(PTE))
// get the length of the pagetable
#define PAGETABLE_LEN (((Entry*)PAGETABLE - 1)->size / sizeof(PTE*))
#define SYS_NUM_PAGES (NUM_PAGES / 2)
#define TEMP_PAGE (GET_PAGE_ADDRESS(NUM_PAGES - SYS_NUM_PAGES))
#define THREAD_NUM_PAGES (NUM_PAGES - SYS_NUM_PAGES)
#define SHALLOC_NUM_PAGES (4)
#define SHALLOC_START_PAGE (THREAD_NUM_PAGES)
#define SYS_PAGE_START ((NUM_PAGES - SYS_NUM_PAGES) + SHALLOC_NUM_PAGES)

char *allmem;

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
    // TODO: unused
    int cur_idx;
    int parent; // for multipage requests, idx of first page
} Page;

// Sub-data structure within a page

typedef struct Entry_{
    size_t size;
    struct Entry_ * next;
    int is_free;
} Entry;

// TODO: add a field for if in swap file
typedef struct Page_Table_Entry_{
    // page_table[x] == all of the Page_Table_Entry's with tcb_id x
    int page_index; // virtual address
    int page_loc; // physical address
    struct Page_Table_Entry_ * next; //TODO: do we need?
} PTE;

typedef struct SysInfo_ {
    PTE* *pagetable;
    Page *mdata;
} SysInfo;

/* Tools */
int my_ceil(double num);

/* mprotect handler */

void handler(int sig, siginfo_t *si, void *unsused);
// load all of the pages of a tcb id into memory
void load_pages(int id);
// check if all of an id's pages are loaded into memory
int check_loaded_pages(int id);

/* Page operations */

// clear id, is_free, mem_free, idx, page_loc, parent, front
void clear_page(Page * curr);
// mprotect all pages with id=id with the flag=protect= 0 or 1 
//      (PROT_NONE = 0; PROT_READ|PROT_WRITE=1)
void my_chmod(int id, int protect);
void single_chmod(int idx, int protect);
void init_front(Page *p);
int find_empty_page();

/* Page table */
void remove_PTE(int id);
int has_PTE(int id, int idx);
void resize_pagetable(int len);
void add_PTE(int id, int idx, int location);
void set_PTE_location(int id, int idx, int location);
int can_access_page(Page * p);

/* Entry operations */
Entry *get_prev_entry(Page *p, Entry *e);
void coalesce(Entry *e, Entry *prev);
void split(Entry *e, int size);
Entry *find_mementry_for_data(Page *p, void* data);

/* Memory init */
void *create_mdata();
void *create_pagetable(void * end_of_mdata);
void mem_init();

/* malloc */
void *single_page_malloc(int size, int id);
void *multi_page_malloc(int req_pages, int size, int id);


int is_multipage_malloc(Page *p);

void *single_page_shalloc(int size);
void *multi_page_shalloc(int req_pages, int size);

#endif
