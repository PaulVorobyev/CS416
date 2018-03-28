#include <signal.h>

#include "./virtual_memory.h"
#include "../asst1/my_pthread_t.h"

char *allmem;
FILE *swapfile = NULL;
int is_initialized = 0;

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
            printf("\tpage_index=%d, page_loc=%d, in_swap=%d\n", pte->page_index, pte->page_loc, pte->in_swap);

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
        Page *p =&MDATA[NUM_PAGES + i];
        printf("\nPAGE #%d\n", NUM_PAGES + i);
        printf("page info: id=%d, is_free=%d, idx=%d, parent=%d, cur_idx=%d\n", p->id, p->is_free, p->idx, p->parent, p->cur_idx);

        p =&MDATA[THREAD_NUM_PAGES];
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

void handler(int sig, siginfo_t *si, void *unused) {
    intptr_t offset = (intptr_t)si->si_addr - (intptr_t)((void*) allmem);
    int page_num = offset / PAGE_SIZE;
    printf("\nERROR TRYING TO ACCESS PAGE #%d as thread %d\n", page_num, get_curr_tcb_id());
    exit(1);
}

void *create_mdata(){
    printf("Creating page meta data \n");
    int pages_needed = MDATA_NUM_PAGES;

    printf("NUM PAGES FOR MDATA %d\n", pages_needed);

    int i;
    int mdata_start_idx = NUM_PAGES-pages_needed;
    // Start mdata at totalnumpages-(mdata)

    printf("num pages: %d          mdata size: %lu\n", MDATA_NUM_PAGES, MDATA_SIZE);
    // make an Entry for mdata
    Entry *mdata_entry = (Entry*) (allmem + ((NUM_PAGES - pages_needed) * PAGE_SIZE));//((char*)SYSINFO + PAGE_SIZE);

    Page * root = (Page*) (mdata_entry + 1);
    Page * curr_page = root;
    Page * prev_page = NULL;
    
    // Populate the page metadata (in OS land) with all,empty pages
    for(i = 0; i < NUM_PAGES; i++){
        // mdata
        if (i >= mdata_start_idx) {
            curr_page->id = 0;
            curr_page->is_free = 0;
            curr_page->mem_free = PAGE_SIZE;
            curr_page->next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE);
            curr_page->prev = prev_page;
            curr_page->front = GET_PAGE_ADDRESS(i);
            curr_page->idx = i;
            curr_page->cur_idx = i;
            curr_page->parent = NUM_PAGES - MDATA_NUM_PAGES;
        } else if (i == mdata_start_idx-1) { // new page where other sys stuff goes
            curr_page->id = 0;
            curr_page->is_free = 0;
            curr_page->mem_free = PAGE_SIZE;
            curr_page->next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE);
            curr_page->prev = prev_page;
            curr_page->front = GET_PAGE_ADDRESS(i);
            curr_page->idx = -1;
            curr_page->cur_idx = i;
            curr_page->parent = i;
        } else {
            curr_page->id = -1;
            curr_page->is_free = 1;
            curr_page->mem_free = PAGE_SIZE;
            curr_page->next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE);
            curr_page->prev = prev_page;
            curr_page->front = GET_PAGE_ADDRESS(i);
            curr_page->idx = -1;
            curr_page->cur_idx = i;
            curr_page->parent = -1;
        }

        if (i <= mdata_start_idx) {
            init_front(curr_page);
            if (i == mdata_start_idx || i == (mdata_start_idx - 1)) {
                curr_page->front->is_free = 0;
            }
        }

        prev_page = curr_page;
        curr_page = curr_page->next;
    }

    // init mdata for swap file
    for (; i < NUM_PAGES + NUM_SWAPFILE_PAGES; i++) {
        curr_page->id = -1;
        curr_page->is_free = 1;
        curr_page->mem_free = PAGE_SIZE;
        curr_page->next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE);
        curr_page->prev = prev_page;
        curr_page->front = GET_PAGE_ADDRESS(i);
        curr_page->idx = -1;
        curr_page->cur_idx = i;
        curr_page->parent = -1;
    }

    // init swapfile pages
    for (i = 0; i < NUM_SWAPFILE_PAGES; i++) {
        int swap_offset = i * PAGE_SIZE;

        // from temp to swap
        fseek(swapfile, swap_offset, SEEK_SET);
        int read = fwrite(GET_PAGE_ADDRESS(0), PAGE_SIZE, 1, swapfile);
    }

    *mdata_entry = (Entry) {
        .size = MDATA_SIZE,
            .next = NULL,
            .is_free = 0 };
    curr_page->next = NULL;

    return (void *)((char *)SYSINFO - sizeof(Entry));
}

void *create_pagetable(void * end_of_mdata){
    printf("Creating page table and SysInfo \n");
    int prev_remaining_space = PAGE_SIZE - sizeof(Entry);

    // SysInfo
    Entry *sys_info_entry = ((Entry*)end_of_mdata);
    *sys_info_entry = (Entry) {
        .size = sizeof(SysInfo),
        .is_free = 0,
        .next = (Entry*) (((char*)(sys_info_entry + 1)) + sizeof(SysInfo)) };
    SysInfo *info = (SysInfo*) (sys_info_entry + 1);
    *info = (SysInfo) {
        .pagetable = (PTE**) (sys_info_entry->next + 1),
        .mdata = (Page*) ((char *)SYSINFO + PAGE_SIZE) };

    // page_table outer
    Entry *page_table_outer_entry = sys_info_entry->next;
    *page_table_outer_entry = (Entry) {
        .size = sizeof(PTE*),
        .is_free = 0,
        .next = (Entry*) (((char*)(page_table_outer_entry + 1)) + sizeof(PTE*)) };
    PTE **page_table_outer = (PTE**) (page_table_outer_entry + 1);
    *page_table_outer = (PTE*) (((char*)(page_table_outer + 1)) + sizeof(Entry));

    // page_table inner
    // I set this to 1 becuase sys actually doesnt need any PTE's
    // However, im afraid to set it to 0 because things might break :(
    int num_sys_pages = 1;
    Entry *page_table_inner_entry = page_table_outer_entry->next;
    *page_table_inner_entry = (Entry) {
        .size = sizeof(PTE) * num_sys_pages,
        .is_free = 0,
        .next = (Entry*) (((char*)(page_table_inner_entry + 1)) + (sizeof(PTE) * num_sys_pages)) };
    int i = 0;
    for (; i < num_sys_pages; i++) {
        page_table_outer[0][i] = (PTE) {
            .page_index = i,
            .page_loc = i,
            .next = (i < num_sys_pages - 1) ? (PTE*) (&page_table_outer[0][0] + (i+1)) : NULL };
    }

    // remaining space entry
    Entry *remaining_entry = page_table_inner_entry->next;
    int size_of_everything_we_made = sizeof(SysInfo) + sizeof(Entry) + sizeof(PTE*) + sizeof(Entry) + sizeof(PTE) + sizeof(Entry);
    *remaining_entry = (Entry) {
        .size = prev_remaining_space - size_of_everything_we_made,
        .is_free = 1,
        .next = NULL };

    return (void*)(remaining_entry + 1);
}

void make_swapfile() {
    int num_bytes = SWAPFILE_SIZE;
    swapfile = fopen("swapfile", "w+");
    fseek(swapfile, num_bytes, SEEK_SET);
    fputc('\0', swapfile);
}

void mem_init(){
    posix_memalign((void**)&allmem, PAGE_SIZE, ARRAY_SIZE);
    make_swapfile();
    void * end_of_mdata = create_mdata();
    create_pagetable(end_of_mdata);

    //register segfault handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        printf("ERROR CANT SETUP SEGFAULT HANDLER\n");
        exit(1);
    }
}

