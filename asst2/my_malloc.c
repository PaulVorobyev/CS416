#include <string.h>

#include "my_malloc.h"
#include "../asst1/my_pthread_t.h"
#include "./virtual_memory.h"

/* Globals */
extern int is_initialized;
extern FILE *swapfile;

/* Misc */

int my_ceil(double num){
    if (num == (int)num) {
        return (int)num;
    }
    return (int)num + 1;
}

void load_pages(int id) {
    printf("LOAD PAGES %d\n", id);
    if(id >= PAGETABLE_LEN){
        return;
    }
    PTE *cur = PAGETABLE[id];
    while (cur) {
        if (cur->page_index != cur->page_loc) {
            printf("PAGE #%d NOT IN PROPER SPOT FOR THREAD #%d", cur->page_index, id);
            swap_pages(cur->page_index, cur->page_loc);
        }

        single_chmod(cur->page_index, 0);
        cur = cur->next;
    }
}

int check_loaded_pages(int id) {
    printf("CHECK LOAD PAGES %d\n", id);
    PTE *cur = PAGETABLE[id];

    while (cur) {
        if (cur->page_index != cur->page_loc) {
            return 0;
        }

        cur = cur->next;
    }

    return 1;
}

int find_page(int id, int size) {
    int i = 0;
    int start = id ? 0 : SYS_PAGE_START;
    int end = id ? THREAD_NUM_PAGES : (NUM_PAGES - MDATA_NUM_PAGES);

    for (i = start; i < end; i++) {
        printf("\nSINGLE MALLOC SEARCHING PAGE#%d FOR THREAD#%d\n", i, id);
        Page *cur = &MDATA[i];

        // cant use multipage malloc'd page
        if (is_multipage_malloc(cur)) {
            printf("\nCANT USE PAGE %d, PART OF MULTIPAGE\n", i);
            continue;
        }

        // if page belongs to someone else, we cant use it, so swap it out
        if (!is_availible_page(cur, id)){
            printf("\nWE ARE GONNA MOVE PAGE %d for %d\n", i, id);
            int empty = find_empty_page(cur->cur_idx);

            if (empty != -1) {
                swap_pages (i, empty);
                single_chmod(i, 0);
            } else {
                int empty_swap = find_empty_swapfile_page();

                if (empty_swap == -1) {
                    return -1;
                }
                
                swap_pages_swapfile(i, empty_swap);
            }
        }

        if (find_mementry(cur->front, size)) {
            printf("\nFOUND A MEMENTRY AT INDEX %d for %d\n", i, id);
            return i;
        }
    }

    return -1;
}

int find_pages(int id, int req_pages, int size) {
    int i = 0;
    int j = 0;

    int start = id ? 0 : SYS_PAGE_START;
    int end = id ? THREAD_NUM_PAGES : (NUM_PAGES - MDATA_NUM_PAGES);
        
    for (i = start; i < end; i++) {
        printf("MULTI MALLOC SEARCHING PAGE#%d FOR THREAD#%d", i, id);
        int all_free = 1;

        for (j = 0; j < req_pages; j++) {
            if ((i + j) >= end) {
                printf("\nOUT OF BOUNDS\n");
                return -1;
            }

            // cur page
            Page *cur = &MDATA[i + j];

            // cant use multipage malloc'd page
            if (is_multipage_malloc(cur)) {
                printf("\nCANT USE PAGE %d, PART OF MULTIPAGE\n", i + j);
                all_free = 0;
                break;
            }

            // if its someone else's, move it or swap
            if (!is_availible_page(cur, id)){
                printf("\nWE ARE GONNA MOVE PAGE %d for %d\n", i, id);
                int empty = find_empty_page(cur->cur_idx);
                if (empty != -1) {
                    swap_pages (i + j, empty);
                    single_chmod(i, 0);
                } else {
                    int empty_swap = find_empty_swapfile_page();

                    if (empty_swap == -1) {
                        return -1;
                    }

                    swap_pages_swapfile(i, empty_swap);
                }
            }

            // all req_pages must be full and free
            if (!page_is_empty(cur)) {
                all_free = 0;
                break;
            }
        }

        if (all_free) {
            return i;
        }
    }

    return -1;
}

/* Malloc */

void *single_page_malloc(int size, int id) {
    printf("single_page_malloc() for %d\n", id);

    int idx = find_page(id, size);

    if (idx == -1) {
        printf("CANT GET A PAGE FOR %d", id);
        return NULL;
    }

    printf("found an entry\n");

    Page *p = &MDATA[idx];

    init_page(p, id, idx, idx);

    Entry *e = find_mementry(p->front, size);

    if (can_be_split(e, size)) split(e, size);
    e->is_free = 0;

    // set PTE
    if (id != 0 && !has_PTE(id, idx)) {
        add_PTE(id, idx, idx);
    }

    return (void*) (e + 1);
}

void *multi_page_malloc(int req_pages, int size, int id) {
    int idx = find_pages(id, req_pages, size);

    if (idx == -1) return NULL;

    int i = 0;
    for (; i < req_pages; i++) {
        Page *cur = &MDATA[idx + i];

        if (i == 0) {
            init_page(cur, id, idx, idx);
            cur->front->size = size;
        } else {
            init_page(cur, id, idx, idx + i);
        }
        
        // mark as taken
        cur->front->is_free = 0;

        // set PTE
        if (id != 0 && !has_PTE(id, idx + i)) {
            add_PTE(id, idx + i, idx + i);
        }
    }

    return (void*) (MDATA[idx].front + 1);
}

void * mymalloc(size_t size, const char * file, int line, int flag) {
    disableAlarm();

    if ((int)size <= 0){
        // fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);
        return NULL;
    }

    if (!is_initialized) {
        mem_init();
        is_initialized = 1;
    }

    int size_with_entry = size + sizeof(Entry);
    int id = get_id(flag);

    printf("\n -------------- Start Malloc for thread #%d for size %d at %s:%d--------------- \n", id, (int)size, file, line);

    // the total number of requested pages
    int req_pages = my_ceil((double)size_with_entry / (double)PAGE_SIZE);

    void *data = (req_pages == 1) ? single_page_malloc(size, id)
        : multi_page_malloc(req_pages, size, id);

    print_mem(flag);
    print_swapfile();
    print_pagetable();

    if (is_sched_init() && !is_in_lib()) {
        printf("\nMALLOC SET ALARM\n");
        setAlarm();
    }

    return data;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    disableAlarm();
    set_in_lib(1);

    intptr_t offset = (intptr_t)ptr - (intptr_t)((void*) allmem);
    int page_num = offset / PAGE_SIZE;
    printf("~~~~~~~~~~~~ Freeing page #%d~~~~~~~~~~~~`\n", page_num);

    if (page_num < 0 || page_num > NUM_PAGES) {
        printf("ERROR: Invalid pointer given to free. %s:%d", file, line);
        exit(1);
    }

    Page *p = &MDATA[page_num];
    Entry *e = find_mementry_for_data(p, ptr);
    //printf("free size: %d\n", (int)e->size);

    if (!e) {
        printf("ERROR: Invalid pointer given to free. %s:%d", file, line);
        exit(1);
    }

    if (e->is_free) {
        printf("ERROR: Attempting to double free. %s:%d", file, line);
        exit(1);
    }

    if (is_multipage_malloc(p)) {
        //TODO: change parent and idx = -1
        printf("is multi page\n");
        Page *cur_p = p;
        cur_p->front->size = PAGE_SIZE - sizeof(Entry);
        while (cur_p->parent == p->idx) {
            cur_p->is_free = 1;
            cur_p->parent = cur_p->idx;
            cur_p->front->is_free = 1;
            
            cur_p = cur_p->next;
        }
    } else {
        e->is_free = 1;
        Entry *prev = get_prev_entry(p, e);
        coalesce(e, prev);
    }

    print_mem(get_id(flag));
    print_pagetable();

    /* printf("\nEND FREE\n"); */
    if (is_sched_init() && !is_in_lib()) {
        printf("\nFREE SET ALARM\n");
        setAlarm();
    }
    set_in_lib(0);
}

