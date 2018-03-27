#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>

/* Page */
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

int is_multipage_malloc(Page *p);
int find_empty_swapfile_page();
int find_empty_page(int start);
void init_page(Page *p, int id, int parent, int idx);
int page_not_owned(Page *p);
int page_is_empty(Page* p);
int is_availible_page(Page *p, int id);
void init_front(Page *p);
void my_chmod(int id, int protect);
void single_chmod(int idx, int protect);
void clear_page(Page * curr);
void swap_pages(int a, int b);
void swap_pages_swapfile(int mem, int swap);
int can_access_page(Page * p);

/* Entry */
typedef struct Entry_{
    size_t size;
    struct Entry_ * next;
    int is_free;
} Entry;

Entry *find_mementry_for_data(Page *p, void* data);
Entry *find_mementry(Entry *front, int size);
int can_be_split(Entry *e, int size);
void split(Entry *e, int size);
void coalesce(Entry *e, Entry *prev);
Entry *get_prev_entry(Page *p, Entry *e);
void fix_entry(Page *p);

/* Page Table Entry */
// TODO: add a field for if in swap file
typedef struct Page_Table_Entry_{
    // page_table[x] == all of the Page_Table_Entry's with tcb_id x
    int page_index; // virtual address
    int page_loc; // physical address
    struct Page_Table_Entry_ * next; //TODO: do we need?
} PTE;

/* Sys Info */
typedef struct SysInfo_ {
    PTE* *pagetable;
    Page *mdata;
} SysInfo;

#endif
