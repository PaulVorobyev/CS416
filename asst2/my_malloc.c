#include <string.h>

#include "my_malloc.h"
#include "../asst1/my_pthread_t.h"
#include "./virtual_memory.h"

/* Globals */
extern int is_initialized;
extern FILE *swapfile;
extern int mallocing;

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
        if (cur->in_swap) {
            swap_pages_swapfile(cur->page_index, NUM_PAGES + cur->page_loc);
        } else if (cur->page_index != cur->page_loc) {
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
        if ((cur->in_swap) || (cur->page_index != cur->page_loc)) {
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

        // if page belongs to someone else, we cant use it, so swap it out
        if (!is_availible_page(cur, id)){
            printf("\nWE ARE GONNA MOVE PAGE %d for %d\n", i, id);
            int empty = find_empty_page(0);

            if (empty != -1) {
                swap_pages (i, empty);
                single_chmod(i, 0);
            } else {
                int empty_swap = find_empty_swapfile_page(2048);

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
        int prev_empty_swap = 2047;

        for (j = 0; j < req_pages; j++) {
            if ((i + j) >= end) {
                printf("\nOUT OF BOUNDS\n");
                return -1;
            }

            Page *cur = &MDATA[i + j];

            // if its someone else's, and we cant move it or swap, give up
            if (!is_availible_page(cur, id) && (find_empty_page(i+j) == -1)) {
                if ((prev_empty_swap = find_empty_swapfile_page(prev_empty_swap + 1)) != -1) {
                    continue;
                }

                all_free = 0;
                break;
            }

            // all req_pages must be full and free
            if (!page_is_empty(cur)) {
                all_free = 0;
                break;
            }
        }

        if (all_free) {
            int hack = 2047;
            // do whatever swapping/shuffling is needed and return
            // the index of the first page in the group
            for (j = 0; j < req_pages; j++) {
                Page *cur = &MDATA[i + j];

                if (!is_availible_page(cur, id)){
                    printf("\nWE ARE GONNA TRY TO MOVE PAGE %d for %d\n", i+j, id);
                    int empty = find_empty_page(i + j);
                    if (empty != -1) {
                        swap_pages (i + j, empty);
                        single_chmod(i+j, 0);
                    } else {
                        swap_pages_swapfile(i+j, hack = find_empty_swapfile_page(hack + 1));
                    }
                }
            }

            return i;
        }
    }

    return -1;
}

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

    printf("multimalloc giving page %d for %d for %d", idx, id, req_pages);
    return (void*) (MDATA[idx].front + 1);
}

void * mymalloc(size_t size, const char * file, int line, int flag) {
    int was_mallocing = mallocing;
    mallocing = 1;

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

    if (!was_mallocing) mallocing = 0;
    return data;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    int was_mallocing = mallocing;
    mallocing = 1;

    intptr_t offset = (intptr_t)ptr - (intptr_t)((void*) allmem);
    int page_num = offset / PAGE_SIZE;
    int id = get_id(flag);
    printf("~~~~~~~~~~~~ Freeing page #%d as %d~~~~~~~~~~~~`\n", page_num, id);

    if (page_num < 0 || page_num > NUM_PAGES) {
        printf("\nERROR: Invalid pointer given to free. %s:%d\n", file, line);
    }
    Page *p = &MDATA[page_num];

    Entry *e = find_mementry_for_data(p, ptr);
    //printf("free size: %d\n", (int)e->size);

    if (!e) {
        printf("\nERROR: Invalid pointer given to free. %s:%d\n", file, line);
    }

    if (e->is_free) {
        printf("\nERROR: Attempting to double free. %s:%d\n", file, line);
    }

    if (is_multipage_malloc(p)) {
        int parent_id = p->id;
        int parent_idx = p->idx;
        Page *cur_p = p;
        while (cur_p->id == parent_id && cur_p->parent == parent_idx) {
            cur_p->is_free = 1;
            cur_p->parent = cur_p->idx;
            cur_p->front->is_free = 1;
            cur_p->front->size = MAX_ENTRY_SIZE;
            
            if (id != 0 && page_is_empty(cur_p)) {
                int cur_idx = cur_p->idx;
                printf("bruh %d %d", id, (int)cur_p->id);
                clear_page(cur_p);
                remove_PTE(id, cur_idx);
            }

            cur_p = cur_p->next;
        }
                printf("bruh %d %d %d", id, (int)cur_p->id, MDATA[1].id);
    } else {
        e->is_free = 1;
        Entry *prev = get_prev_entry(p, e);
        coalesce(e, prev);

        if (id != 0 && page_is_empty(p)) {
            int idx = p->idx;
            clear_page(p);
            remove_PTE(id, idx);
        }
    }

    print_mem(get_id(flag));
    print_pagetable();

    /* printf("\nEND FREE\n"); */
    if (is_sched_init() && !is_in_lib()) {
        printf("\nFREE SET ALARM\n");
        setAlarm();
    }

    if (!was_mallocing) mallocing = 0;
}

