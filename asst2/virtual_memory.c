#include "./virtual_memory.h"
#include "../asst1/my_pthread_t.h"

/* colors for printing */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int get_id(int flag){
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
    
    return id;
}

void print_pagetable() {
    printf("\n############### CURRENT PAGETABLE LAYOUT ###############\n");

    int i = 0;
    for (; i < PAGETABLE_LEN; i++) {
        PTE* ptes = PAGETABLE[i];
        printf("PTES for Thread #%d:\n", i);

        PTE *pte = &ptes[0];
        while (pte) {
            printf("\tpage_index=%d, page_loc=%d\n", pte->page_index, pte->page_loc);

            pte = pte->next;
        }
    }
}

void print_mem(int flag){
    printf("############### CURRENT MEMORY LAYOUT ###############\n");

    int i = 0;
    for (; i < THREAD_NUM_PAGES + 1; i++) {
        int id = get_curr_tcb_id();
        // main() tcb_id = -1 in scheduler && id=1 in mmu
        id = (id != -1) ? id : 1; 

        Page *p = &MDATA[i];
        if (!can_access_page(p)) {
            single_chmod(i, 0);
        }

        printf("\nPAGE #%d\n", i);

        if (GET_PAGE_ADDRESS(i) == TEMP_PAGE) {
            printf("TEMP PAGE\n");
            continue;
        }

        printf("page info: id=%d, is_free=%d, idx=%d, parent=%d, cur_idx=%d\n", p->id, p->is_free, p->idx, p->parent, p->cur_idx);

        Entry *e = p->front;
        //printf("\nFRONT %p\n", p->front);

        if (p->parent != -1 && p->parent != p->idx) {
            printf(ANSI_COLOR_RED "\tPART OF MULTIPAGE MALLOC\n" ANSI_COLOR_RESET);
            continue;
        }

        while (e) {
            printf(e->is_free ? ANSI_COLOR_GREEN : ANSI_COLOR_RED);
            printf("\tentry info: size=%lu, is_free=%d\n" ANSI_COLOR_RESET, e->size, e->is_free);
            e = e->next;
        }


        if (!can_access_page(p)) {
            single_chmod(i, 1);
        }
        
    }
}

void print_swapfile() {
    int pages = 5;
    int i = 0;

    for (; i < pages; i++){
        rewind(swapfile);
        fseek(swapfile, i * PAGE_SIZE, SEEK_SET);
        fread(TEMP_PAGE, PAGE_SIZE, 1, swapfile);
        fix_entry(&MDATA[ THREAD_NUM_PAGES ]);
        Page *p =&MDATA[THREAD_NUM_PAGES];
        printf("\nPAGE #%d\n", NUM_PAGES + i);
        printf("page info: id=%d, is_free=%d, idx=%d, parent=%d, cur_idx=%d\n", p->id, p->is_free, p->idx, p->parent, p->cur_idx);

        Entry *e = p->front;
        //printf("\nFRONT %p\n", p->front);

        if (p->parent != -1 && p->parent != p->idx) {
            printf("\tPART OF MULTIPAGE MALLOC\n" );
            continue;
        }

        while (e) {
            printf("\tentry info: size=%lu, is_free=%d\n", e->size, e->is_free);
            e = e->next;
        }
    }
}

