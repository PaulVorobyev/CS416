//#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "my_malloc.h"
#include "virt_mem.h"

/* Forward Declarations */

Entry *find_mementry(Entry *front, int size);
void swap_pages(int a, int b);
int printing_page = -1;
int handler_used = 0;

/* Misc */

// SCREW PRINTING
void check_if_used_handler(){
    if (handler_used){
        mprotect(GET_PAGE_ADDRESS(printing_page), PAGE_SIZE, PROT_NONE);
        handler_used = 0;
    }
}

void set_printing_page(int idx){
    printing_page = idx;
}

void clear_printing_page(){
    printing_page = -1;
}

int my_ceil(double num){
    if (num == (int)num) {
        return (int)num;
    }
    return (int)num + 1;
}

/* mprotect handler */

void handler(int sig, siginfo_t *si, void *unused) {
    printf("hello from segfault handler\n");

    // handle printing
    if (printing_page > -1){
        printf("printing\n");
        mprotect(GET_PAGE_ADDRESS(printing_page), PAGE_SIZE, PROT_READ);
        handler_used = 1;
        return;
    }

    if (check_loaded_pages(get_curr_tcb_id())) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_FAILURE);
}

void load_pages(int id) {
    printf("LOAD PAGES %d\n", id);
    PTE *cur = PAGETABLE[id];

    while (cur) {
        if (cur->page_index != cur->page_loc) {
            swap_pages(cur->page_index, cur->page_loc);

            cur->page_loc = cur->page_index; 
        }

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

/* Page Operations */

void swap_pages(int a, int b) {
    printf("\nSWAP PAGES %d and %d\n", a, b);
    void *a_loc = GET_PAGE_ADDRESS(a);
    void *b_loc = GET_PAGE_ADDRESS(b);

    memcpy(TEMP_PAGE, a_loc, PAGE_SIZE);
    memcpy(a_loc, b_loc, PAGE_SIZE);
    memcpy(b_loc, TEMP_PAGE, PAGE_SIZE);
}

void clear_page(Page * curr){
    curr->id = -1;
    curr->is_free = 1;
    curr->mem_free = PAGE_SIZE;
    curr->front->size = MAX_ENTRY_SIZE;
    curr->front->next = NULL;
    curr->front->is_free = 1 ;
    curr->idx = -1;
    curr->parent = -1;
}

void my_chmod(int id, int protect) {
    printf("%sing thread #%d", protect ? "protect" : "unprotect", id);
    if (id >= PAGETABLE_LEN) {
        printf("CHMOD: THREAD#%d NOT IN PAGETABLE YET\n", id);
        return;
    }

    // take the tcb_id and protect flag (1 for change all pages to PROT_NONE, 0 for inverse)
    PTE *cur = PAGETABLE[id];
    while (cur){
        // TODO: check if page is in mem and not swap file
        if (1) {
            mprotect(GET_PAGE_ADDRESS(cur->page_loc), PAGE_SIZE, protect ? PROT_NONE : PROT_READ|PROT_WRITE); 
        }

        cur = cur->next;
    }
}

void init_front(Page *p) {
    *p->front = (Entry) {
        .size = MAX_ENTRY_SIZE,
        .next = NULL,
        .is_free = 1 };
}

int is_availible_page(Page *p, int id) {
    return (p->id == -1 || p->id == id) && (p->parent == -1 || p->parent == p->idx);
}

int page_is_empty(Page* p) {
    //printf("IDX: %d\n", p->idx);
    //printf("ID: %d\n", p->id);
    //printf("%d\n", p->front ? 777 : 666);
    //printf("%p\n", p->front->next);
    return p->front->is_free && !p->front->next;
}

void init_page(Page *p, int id, int idx, int parent) {
    p->id = id;
    p->is_free = 0;
    p->idx = idx;
    p->parent = parent;
}

int find_page(int id, int size) {
    int i = 0;
    int start = id ? 0 : THREAD_NUM_PAGES;
    int end = id ? THREAD_NUM_PAGES : (NUM_PAGES - MDATA_NUM_PAGES);

    printf("\nWTF id=%d start=%d end=%d\n", id, start, end);

    for (i = start; i < end; i++) {
        Page *cur = &MDATA[i];

        // if page belongs to someone else, we cant use it
        if (!is_availible_page(cur, id)) continue;

        if (find_mementry(cur->front, size)) {
            return i;
        }
    }

    return -1;
}

int find_pages(int id, int req_pages, int size) {
    int i = 0;
    int j = 0;

    int start = id ? 0 : THREAD_NUM_PAGES;
    int end = id ? THREAD_NUM_PAGES : (NUM_PAGES - MDATA_NUM_PAGES);
        
    for (i = start; i < end; i++) {
        int all_free = 1;

        for (j = 0; j < req_pages; j++) {
            // cur page
            Page *cur = &MDATA[i + j];

            // if page belongs to someone else, we cant use it
            if (!is_availible_page(cur, id)) {
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

int is_multipage_malloc(Page *p) {
    return (!p->front->next) && (!p->front->is_free) && (p->next) && (p->next->parent == p->idx);
}

/* Pagetable Operations */

// for when a tcb terminates/finishes
void remove_PTE(int id){
    printf("Clearing PTE data of of THREAD #%d\n", id);
    // Clear pages in mdata
    int i;
    for (i = 0; i < THREAD_NUM_PAGES; i++) {
        Page *cur = &MDATA[i];
        if (cur->id == id){
            clear_page(cur);
        }
    }

    my_chmod(id, 0);

    // Free PTEs
    PTE *curr_pte = PAGETABLE[id];
    PTE *next = NULL;
    PAGETABLE[id] = NULL;
    do{
        next = curr_pte->next;
        myfree((void*) curr_pte, __FILE__, __LINE__, LIBRARYREQ);
        curr_pte = next;
    }while(curr_pte);
}

int has_PTE(int id, int idx) {
    if (id >= PAGETABLE_LEN) return 0;

    printf("IDX: %d", id);

    PTE* cur = PAGETABLE[id];
    while (cur && cur->page_index != idx) cur = cur->next;

    return cur ? 1 : 0;
}

void resize_pagetable(int len) {
    int prev_len = PAGETABLE_LEN;
    PTE **prev_pt = PAGETABLE;

    // copy old pt outer to new spot and update sysinfo
    // i use 1 here because I dont have scope of LIBRARYREQ
    PTE **pt = (PTE**) mymalloc(len * sizeof(PTE*), __FILE__, __LINE__, 1);
    memcpy((void*)pt, (void*)SYSINFO->pagetable, PAGETABLE_LEN * sizeof(PTE*));
    SYSINFO->pagetable = pt;

    // init any new threads in the array to NULL
    int i = prev_len;
    for (; i < len; i++) PAGETABLE[i] = NULL;

    free((void*) prev_pt);
}

void add_PTE(int id, int idx, int location) {
    printf("\nADD PTE %d\n", id);
    // make sure thread has an array in our pagetable
    if (id >= PAGETABLE_LEN) {
        resize_pagetable(id + 1);
    }

    // if array is null, init it with size of 1 and add the PTE
    if (!PAGETABLE[id]) {
        // i use 1 here because I dont have scope of LIBRARYREQ
        PAGETABLE[id] = (PTE*) mymalloc(sizeof(PTE), __FILE__, __LINE__, 1);
        *PAGETABLE[id] = (PTE) {
            .page_index = idx,
            .page_loc = location,
            .next = NULL };
    } else { // add to end
        PTE* cur = PAGETABLE[id];

        // goto last entry
        while (cur->next) cur = cur->next;

        // i use 1 here because I dont have scope of LIBRARYREQ
        cur->next = (PTE*) mymalloc(sizeof(PTE), __FILE__, __LINE__, 1);
        *cur->next = (PTE) {
            .page_index = idx,
            .page_loc = location,
            .next = NULL };
    }
}

void set_PTE_location(int id, int idx, int location) {
    PTE *ptes = PAGETABLE[id];
    PTE *cur = &ptes[0];

    while (cur) {
        if (cur->page_index == idx) {
            cur->page_loc = location;
        }

        cur = cur->next;
    }
}

/* Entry Operations */

Entry *get_prev_entry(Page *p, Entry *e) {
    Entry *cur = p->front;
    while (cur->next) {
        if (cur->next == e) return cur;

        cur = cur->next;
    }

    return NULL;
}

void coalesce(Entry *e, Entry *prev) {
    // merge with prev
    if (prev && prev->is_free) {
        prev->size += e->size + sizeof(Entry);
        prev->next = e->next;
        e = prev;
    } 

    // merge with next
    if (e->next && e->next->is_free) {
        e->size += e->next->size + sizeof(Entry);
        e->next = e->next->next;
    }
}

void split(Entry *e, int size) {
    int prev_size = e->size;
    Entry *prev_next = e->next;

    e->size = size;
    e->next = (Entry*) (((char*)(e + 1)) + size);

    // remaining space entry
    *e->next = (Entry) {
        .size = prev_size - (e->size + sizeof(Entry)),
        .next = prev_next,
        .is_free = 1 };
}

int can_be_split(Entry *e, int size) {
    return e->size > (size + sizeof(Entry) + 100);
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

Entry *find_mementry_for_data(Page *p, void* data) {
    Entry *e = p->front;
    while (e) { 
        if ((void*)(e+1) == data) return e;

        e = e->next;
    }

    return NULL;
}

/* Memory Initialization */

void *create_mdata(){
    printf("Creating page meta data \n");
    int pages_needed = my_ceil((double)MDATA_SIZE / (double)PAGE_SIZE);

    printf("NUM PAGES FOR MDATA %d\n", pages_needed);

    int i;
    int mdata_start_idx = NUM_PAGES-pages_needed;
    // Start mdata at totalnumpages-(mdata)

    printf("num pages: %d          mdata size: %d\n", MDATA_NUM_PAGES, MDATA_SIZE);
    // make an Entry for mdata
    Entry *mdata_entry = (Entry*) (allmem + ((NUM_PAGES - pages_needed) * PAGE_SIZE));//((char*)SYSINFO + PAGE_SIZE);

    Page * root = (Page*) (mdata_entry + 1);
    Page * curr_page = root;
    Page * prev_page = NULL;
    
    // Populate the page metadata (in OS land) with all,empty pages
    for(i = 0; i < NUM_PAGES; i++){
        // mdata
        if (i >= mdata_start_idx) {
            *curr_page = (Page) {
                .id = 0,
                .is_free = 0,
                .mem_free = PAGE_SIZE,
                .next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE),
                .prev = prev_page,
                .front = GET_PAGE_ADDRESS(i) ,
                .idx = i,
                .parent = NUM_PAGES - MDATA_NUM_PAGES };
        } else if (i == mdata_start_idx-1) { // new page where other sys stuff goes
            *curr_page = (Page) {
                .id = 0,
                .is_free = 0,
                .mem_free = PAGE_SIZE,
                .idx = i,
                .parent = i,
                .next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE),
                .front = GET_PAGE_ADDRESS(i) };
        } else {
            *curr_page = (Page) {
                .id = -1,
                .is_free = 1,
                .mem_free = PAGE_SIZE,
                .next = (Page *) ( (char *)curr_page + PAGE_STRUCT_SIZE),
                .prev = prev_page,
                .front = GET_PAGE_ADDRESS(i),
                .idx = -1,
                .parent = -1 };
        }

        init_front(curr_page);
        if (i == mdata_start_idx || i == mdata_start_idx - 1) curr_page->front->is_free = 0;
        
        prev_page = curr_page;
        curr_page = curr_page->next;
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
    int num_sys_pages = my_ceil((double)MDATA_SIZE / (double)PAGE_SIZE) + 1;
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

void mem_init(){
    printf("MEM INIT\n");

    posix_memalign((void**)&allmem, PAGE_SIZE, ARRAY_SIZE);

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

/* Malloc */

void *single_page_malloc(int size, int id) {
    printf("single_page_malloc() for %d\n", id);

    int idx = find_page(id, size);

    if (idx == -1) return NULL;

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
            cur->front->is_free = 0;
            cur->front->size = size;
        } else {
            init_page(cur, id, idx + i, idx);
        }

        // set PTE
        if (id != 0 && !has_PTE(id, idx)) {
            add_PTE(id, idx, idx);
        }
    }

    return (void*) (MDATA[idx].front + 1);
}

