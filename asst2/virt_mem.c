#include "virt_mem.h"
#include <stdlib.h>
#include <stdio.h>

void mem_init(){
    printf("MEM INIT\n");
    void * end_of_mdata = create_mdata(allmem);
    create_pagetable(end_of_mdata);

    // claim first page for sys
    MDATA[0] = (Page) {
        .id = 0,
        .is_free = 0,
        .idx = 0,
        .parent = 0,
        .front = (Entry*) allmem };
}

void *create_pagetable(void * end_of_mdata){
    printf("Creating page table and SysInfo \n");
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

void *create_mdata(){
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

void print_mem(){
    printf("############### CURRENT MEMORY LAYOUT ###############");

    int i = 0;
    for (; i < 20; i++) {
        Page p = MDATA[i];

        printf("PAGE #%d", i);
        printf("page info: id=%d, is_free=%d, idx=%d, parent=%d", p.id, p.is_free, p.idx, p.parent);

        Entry *e = p.front;

        if (!e) {
            printf("\tNO ENTRIES");
            continue;
        }

        while (e) {
            printf("\tentry info: size=%d, is_free=%d", e->size, e->is_free);
            e = e->next;
        }
    }
}

void coalesce(Entry *e) {
    if (e->next->next && e->next->next->is_free) {
        e->next->size += e->next->next->size + sizeof(Entry);
        e->next->next = e->next->next->next;
    }
}

void split(Entry *e, int size) {
    int prev_size = e->size;
    Entry *prev_next = e->next;

    *e = (Entry) {
        .size = size - sizeof(Entry),
        .next = (Entry*) (((char*)(e + 1)) + (size - sizeof(Entry))),
        .is_free = 0 };

    // remaining space entry
    *e->next = (Entry) {
        .size = prev_size - (e->size + sizeof(Entry)),
        .next = prev_next,
        .is_free = 1 };
}

int can_be_split(Entry *e, int size) {
    return e->size > (size + sizeof(Entry) + 100);
}

int is_availible_page(Page *p, int id) {
    return (p->id == -1 || p->id == id) && (p->parent == -1 || p->parent == p->idx);
}

int page_is_empty(Page* p) {
    return (!p->front) || (p->front->is_free && !p->front->next);
}

void init_front(Page *p) {
    p->front = (Entry*) (allmem + (PAGE_SIZE * p->idx));
    *p->front = (Entry) {
        .size = PAGE_SIZE - sizeof(Entry),
        .next = NULL,
        .is_free = 1 };
}

void set_parent_page(Page *p, int id, int idx) {
    *p = (Page) {
        .id = id,
        .is_free = 0,
        .idx = idx,
        .parent = idx };
}

void set_child_page(Page *p, int id, int idx, int parent) {
    *p = (Page) {
        .id = id,
        .is_free = 0,
        .idx = idx,
        .parent = parent,
        .front = NULL };
}

Entry *find_mementry(Entry *front, int size) {
    Entry *cur = front;

    while (cur) {
        if (cur->is_free && (cur->size >= size)) {
            return cur;
        }

        cur = cur->next;
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

    set_parent_page(&p, id, idx);

    Entry *e = NULL;
    if (!p.front) {
        init_front(&p);
        e = p.front;
    } else {
        e = find_mementry(p.front, size);
    }

    if (can_be_split(e, size)) {
        split(e, size);
        coalesce(e);
    } else {
        e->is_free = 0;
    }

    return (void*) (e + 1);
}

int find_pages(int id, int req_pages, int size) {
    int i = 0;
    int j = 0;

    for (i = 0; i < NUM_PAGES; i++) {
        int all_free = 1;

        for (j = 0; j < req_pages; j++) {
            // cur page
            Page cur = MDATA[i + j];

            // if page belongs to someone else, we cant use it
            if (!is_availible_page(&cur, id)) continue;

            // all req_pages, excluding last one, must be full and free
            // this can me an empty page or a page with one entry that is free
            if (j < (req_pages - 1)) {
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

void *multi_page_malloc(int req_pages, int size, int id) {
    int idx = find_pages(id, req_pages, size);

    if (idx == -1) return NULL;

    int i = 0;
    for (; i < req_pages; i++) {
        Page cur = MDATA[idx + i];

        if (i == 0) {
            set_parent_page(&cur, id, idx);
            init_front(&cur);
            cur.front->is_free = 0;
        } else {
            set_child_page(&cur, id, idx + i, idx);
        }
    }

    return (void*) (MDATA[idx].front + 1);
}

void *_malloc(int req_pages, int size, int id){
    printf("System malloc\n");

    void *data = (req_pages == 1) ? single_page_malloc(size, id)
        : multi_page_malloc(req_pages, size, id);

    print_mem();

    return data;
}

int ceil(double num){
    if (num == (int)num) {
        return (int)num;
    }
    return (int)num + 1;
}

