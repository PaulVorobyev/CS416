#include <stdio.h>

#include "./my_shalloc.h"
#include "./virtual_memory.h"
#include "./structures.h"
#include "../asst1/my_pthread_t.h"

extern int is_initialized;
extern int mallocing;

int find_page_shalloc(int size) {
    int i = SHALLOC_START_PAGE;

    for (; i < SYS_PAGE_START; i++) {
        printf("\nSINGLE SHALLOC SEARCHING PAGE#%d\n", i);
        Page *cur = &MDATA[i];

        // cant use multipage malloc'd page
        if (is_multipage_malloc(cur)) {
            printf("\nCANT USE PAGE %d, PART OF MULTIPAGE\n", i);
            continue;
        }

        if (find_mementry(cur->front, size)) {
            printf("\nFOUND A MEMENTRY AT INDEX %d\n", i);
            return i;
        }
    }

    return -1;
}

int find_pages_shalloc(int req_pages, int size) {
    int i = SHALLOC_START_PAGE;
    int j = 0;
        
    for (; i < SYS_PAGE_START; i++) {
        printf("MULTI SHALLOC SEARCHING PAGE#%d\n", i);
        int all_free = 1;

        for (j = 0; j < req_pages; j++) {
            if ((i + j) >= SYS_PAGE_START) {
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

void *single_page_shalloc(int size) {
    printf("single_page_shalloc()\n");

    int idx = find_page_shalloc(size);

    if (idx == -1) return NULL;

    printf("found an page\n");

    Page *p = &MDATA[idx];

    init_page(p, -1, idx, idx);

    Entry *e = find_mementry(p->front, size);

    if (can_be_split(e, size)) split(e, size);
    e->is_free = 0;

    return (void*) (e + 1);
}

void *multi_page_shalloc(int req_pages, int size) {
    int idx = find_pages_shalloc(req_pages, size);

    if (idx == -1) return NULL;

    int i = 0;
    for (; i < req_pages; i++) {
        Page *cur = &MDATA[idx + i];

        if (i == 0) {
            init_page(cur, -1, idx, idx);
            cur->front->size = size;
        } else {
            init_page(cur, -1, idx, idx + i);
        }

        // mark as taken
        cur->front->is_free = 0;
    }

    return (void*) (MDATA[idx].front + 1);
}

void *shalloc(size_t size) {
    int was_mallocing = mallocing;
    mallocing = 1;

    if (is_sched_init()) {
        printf("\nMALLOC DISABLE ALARM\n");
        disableAlarm();
    }

    if ((int)size <= 0){
        // fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);
        return NULL;
    }

    if (!is_initialized) {
        mem_init();
        is_initialized = 1;
    }

    int size_with_entry = size + sizeof(Entry);
    int id = get_id(THREADREQ);


    printf("\n -------------- Start Shalloc for thread #%d for size %d--------------- \n", id, (int)size);


    // the total number of requested pages
    int req_pages = my_ceil((double)size_with_entry / (double)PAGE_SIZE);

    void *data = (req_pages == 1) ? single_page_shalloc(size)
        : multi_page_shalloc(req_pages, size);

    //print_mem(THREADREQ);
    //print_pagetable();

    if (is_sched_init()) {
        printf("\nMALLOC SET ALARM\n");
        setAlarm();
    }

    if (!was_mallocing) mallocing = 0;
    return data;
}

