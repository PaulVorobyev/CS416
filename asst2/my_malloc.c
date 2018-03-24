#include <stdlib.h>
#include <stdio.h>

#include "../asst1/my_pthread_t.h"
#include "my_malloc.h"
#include <stdint.h>

/* Defines */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Globals */
int is_initialized = 0;

void print_pagetable() {
    printf("\n############### CURRENT PAGETABLE LAYOUT ###############\n");

    int i = 0;
    for (; i < PAGETABLE_LEN; i++) {
        PTE* ptes = PAGETABLE[i];
        printf("PTES for Thread #%d:\n", i);

        PTE *pte = &ptes[0];
        while (pte) {
            printf("\tpage_index=%d, page_loc=%p\n", pte->page_index, pte->page_loc);

            pte = pte->next;
        }
    }
}

void print_mem(){
    printf("\n############### CURRENT MEMORY LAYOUT ###############\n");

    int i = 0;
    for (; i < 3; i++) {
        Page *p = &MDATA[i];

        printf("PAGE #%d\n", i);
        printf("page info: id=%d, is_free=%d, idx=%d, parent=%d\n", p->id, p->is_free, p->idx, p->parent);

        Entry *e = p->front;

        if (p->parent != -1 && p->parent != p->idx) {
            printf(ANSI_COLOR_RED "\tPART OF MULTIPAGE MALLOC\n" ANSI_COLOR_RESET);
            continue;
        }

        while (e) {
            printf(e->is_free ? ANSI_COLOR_GREEN : ANSI_COLOR_RED);
            printf("\tentry info: size=%lu, is_free=%d\n" ANSI_COLOR_RESET, e->size, e->is_free);
            e = e->next;
        }
    }
}

void * mymalloc(size_t size, const char * file, int line, int flag) {
    disableAlarm();
    printf("Start Malloc\n");

    if ((int)size <= 0){
        // fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);
        return NULL;
    }

    if (!is_initialized) {
        mem_init();
        is_initialized = 1;
    }

    int size_with_entry = size + sizeof(Entry);

    // if its a libraryreq then its sys i.e. 0
    // if not, then check what scheduler's curr is.
    // if thats not -1, then use it, but if it is
    // then it must be the main thread making the request
    // and its id is (will be) 1
    int current_thread = get_curr_tcb_id();
    // 0 = library, 1 = main(), # = tcb_id
    int id = (flag == LIBRARYREQ) ? 0 : 
        (current_thread != -1) ? current_thread : 1; 

    // quick hack for making sure malloc from pthread
    // counts as sys
    if (is_in_lib()) {
        id = 0;
    }

    // the total number of requested pages
    int req_pages = my_ceil((double)size_with_entry / (double)PAGE_SIZE);

    void *data = (req_pages == 1) ? single_page_malloc(size, id)
        : multi_page_malloc(req_pages, size, id);

    print_mem();
    print_pagetable();

    if (is_sched_init()) {
        setAlarm();
    }

    printf("%d", (data) ? 777 : 66);

    return data;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    disableAlarm();

    intptr_t offset = (intptr_t)ptr - (intptr_t)((void*) allmem);
    int page_num = offset / PAGE_SIZE;

    if (page_num < 0 || page_num > NUM_PAGES) {
        printf("ERROR: Invalid pointer given to free. %s:%d", file, line);
        exit(1);
    }

    Page *p = &MDATA[page_num];
    Entry *e = find_mementry_for_data(p, ptr);

    if (!e) {
        printf("ERROR: Invalid pointer given to free. %s:%d", file, line);
        exit(1);
    }

    if (e->is_free) {
        printf("ERROR: Attempting to double free. %s:%d", file, line);
        exit(1);
    }

    if (is_multipage_malloc(p)) {
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

    //print_mem();
    //print_pagetable();

    if (is_sched_init()) {
        setAlarm();
    }
}

