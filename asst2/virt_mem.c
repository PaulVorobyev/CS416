#include "virt_mem.h"
#include <stdlib.h>
#include <stdio.h>

void mem_init(char allmem[]){
    void * end_of_mdata = create_mdata(allmem);
    create_pagetable(allmem, end_of_mdata);

    // claim first page for sys
    MDATA[0] = (Page) {
        .id = 0,
        .is_free = 0,
        .idx = 0,
        .parent = 0,
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
    int mdata_size = sizeof(Page) * NUM_PAGES;

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
    for(i = 0; i < NUM_PAGES; i++){
        *curr_page = (Page) {
            .id = -1,
            .is_free = 1,
            .mem_free = PAGE_SIZE,
            .next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE),
            .prev = prev_page,
            .front = NULL,
            .idx = -1,
            .parent = -1 };
        
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

//TODO: i think my changes have broken this (@bigolu)
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

int can_be_split(Entry *e, int size) {
    return e->size > (size + sizeof(Entry) + 100);
}

int is_availible_page(Page *p, int id) {
    return p->id == -1 || p->id == id;
}

void init_front(Page *p) {
    p->front = (Entry*) (allmem + (PAGE_SIZE * p->idx));
    *p->front = (Entry) {
        .size = PAGE_SIZE - sizeof(Entry),
        .next = NULL,
        .is_free = 1 };
}

void init_page(Page *p, int id, int idx) {
    *p = (Page) {
        .id = id,
        .is_free = 0,
        .idx = idx,
        .parent = idx };
}

Entry *find_mementry(Entry *front, int size) {
    Entry *cur = front;

    while (cur != NULL) {
        if (cur->is_free && (cur->size >= size)) {
            return cur;
        }
    }

    return NULL;
}

int find_page(int id, int size) {
    int i = 0;

    for (i = 0; i < NUM_PAGES; i++) {
        Page cur = MDATA[i];

        // if page belongs to someone else, we cant use it
        if (!is_availible_page(&cur, id)) continue;

        if (page_is_empty(&cur) || find_mementry(cur.front, size)) {
            return i;
        }
    }

    return -1;
}

void *single_page_malloc(int size, int id) {
    int idx = find_page(id, size);

    if (idx == -1) return NULL;

    Page p = MDATA[idx];

    // in this page hasn't been initialized yet
    // if it has this will just be redundant
    init_page(&p, id, idx);

    // set entry
    Entry *e = NULL;
    if (!p.front) {
        init_front(&p);
        e = p.front;
    } else {
        e = find_mementry(p.front, size);
    }

    int prev_size = e->size;
    Entry *prev_next = e->next;

    if (can_be_split(e, size)) {
        *e = (Entry) {
            .size = size - sizeof(Entry),
            .next = (Entry*) (((char*)(e + 1)) + (size - sizeof(Entry))),
            .is_free = 0 };

        // remaining space entry
        *e->next = (Entry) {
            .size = prev_size - (e->size + sizeof(Entry)),
            .next = prev_next,
            .is_free = 1 };

        // try to coalesce
    } else {
        e->is_free = 0;
    }

    return (void*) (e + 1);
}

void *_malloc(int req_pages, int size, int id){
    printf("System malloc\n");

    return (req_pages == 1) ? single_page_malloc(size, id)
        : multi_page_malloc(req_pages, size, id);

    int idx = (req_pages == 1) ? find_page(id, size)
        : find_pages(id, req_pages, size);

    if (idx == -1) return NULL;

    Page p = MDATA[idx];

    // if its empty
    if (page_is_empty(&p)) {
        int i = 0;
        for (; i < req_pages; i++) {
            Page cur = MDATA[idx + i];
            Entry *front = cur.front;

            if (i == 0) {
                cur = (Page) {
                    .id = id,
                    .is_free = 0,
                    .idx = idx,
                    .parent = idx };

                if (!front) {
                    cur.front = (Entry*) (allmem + (PAGE_SIZE * idx));
                    *cur.front = (Entry) {
                        .size = (req_pages == 1) ? size - sizeof(Entry)
                            : PAGE_SIZE - sizeof(Entry),
                        .next = NULL,
                        .is_free = 0 };
                    if (req_pages == 1) {
                        cur.front->next = (Entry*) (((char*)(cur.front + 1)) + cur.front->size);
                        *cur.front->next = (Entry) {
                            .size = PAGE_SIZE - (sizeof(Entry) + cur.front->size + sizeof(Entry)),
                            .next = NULL,
                            .is_free = 1 };
                    }
                } else {
                
                }
            } else {
                cur = (Page) {
                    .id = id,
                    .is_free = 0,
                    .idx = idx + i,
                    .parent = idx,
                    .front = NULL };
            }
        }
    }

        .id = 0,
        .is_free = 0,
        .idx = 0,
        .parent = 0,
        .front = (Entry*) allmem };

    if (req_pages == 1) {
        
    } else {
    
    }


}

void *multi_page_malloc(int req_pages, int size, int id) {

}

int ceil(double num){
    if (num == (int)num) {
        return (int)num;
    }
    return (int)num + 1;
}

int page_is_empty(Page* p) {
    return (!p->front) || (p->front->is_free && !p->front->next);
}

int find_pages(int id, int req_pages, int size) {
    int i = 0;
    int j = 0;

    for (i = 0; i < NUM_PAGES; i++) {
        int all_free = 1;

        for (j = 0; j < req_pages; j++) {
            // idx of cur page
            int k = i+j;
            // cur page
            Page cur = MDATA[k];

            // if page belongs to someone else, we cant use it
            if (cur.id != -1 && cur.id != id) {
                all_free = 0;
                break;
            }

            // first page must have at least PAGE_SIZE - sizeof(Entry)
            if (j == 0) {
                // if its empty, we're good, otherwise we should check
                // amount of free space
                if (!page_is_empty(&cur)) {
                    all_free = 0;
                    break;
                }
            }

            // all req_pages, excluding first/last one, must be full and free
            // this can me an empty page or a page with one entry that is free
            if (j < (req_pages - 1) && j > 0) {
                // if its empty, we're good, otherwise we should check
                // amount of free space
                if (!page_is_empty(&cur)) {
                    all_free = 0;
                    break;
                }
            }

            // last page needs size % PAGE_SIZE
            if (j == req_pages - 1) {
                if (!page_is_empty(&cur)) {
                    Entry *e = find_mementry(cur.front, size % PAGE_SIZE);
                    if (!e) {
                        all_free = 0;
                        break;
                    }
                }
            }
        }

        if (all_free) {
            return i;
        }
    }

    return -1;
}

