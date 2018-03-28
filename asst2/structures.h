#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>

/* Page */
typedef struct Page_{
    int id; // id of TCB; -1 for empty/free page
    int is_free; // TODO: not used
    size_t mem_free; //TODO: not used
    struct Page_ * next;
    struct Page_ * prev;
    struct Entry_ * front;
    int idx;
    int cur_idx;
    int parent; // for multipage requests, idx of first page
} Page;

int is_multipage_malloc(Page *p);
int find_empty_swapfile_page(int start);
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
typedef struct Page_Table_Entry_{
    // page_table[x] == all of the Page_Table_Entry's with tcb_id x
    int page_index; // virtual address
    int page_loc; // physical address
    int in_swap; // 0 = no, 1 = yes
    struct Page_Table_Entry_ * next;
} PTE;

void resize_pagetable(int len);
void remove_PTE(int id, int idx);
int has_PTE(int id, int idx);
void add_PTE(int id, int idx, int location);
void set_PTE_location(int id, int idx, int location, int in_swap);

/* Sys Info */
typedef struct SysInfo_ {
    PTE* *pagetable;
    Page *mdata;
} SysInfo;

#endif
