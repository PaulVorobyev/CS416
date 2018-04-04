#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "./structures.h"
#include "./virtual_memory.h"
#include "../asst1/my_pthread_t.h"

extern FILE *swapfile;

/* Page Table Entry */
void remove_PTE(int id, int idx){
    printf("Clearing PTE data of %d for THREAD #%d\n", idx, id);
    
    if (id >= PAGETABLE_LEN) return;

    PTE *pte = PAGETABLE[id];
    PTE *prev_pte = NULL;

    // find PTE
    while (pte && (pte->page_index != idx)) {
        prev_pte = pte;
        pte = pte->next;
    }

    // remove PTE
    if (pte) {
        if (!prev_pte) {
            PAGETABLE[id] = pte->next;
        } else {
            prev_pte->next = pte->next;
        }
        //myfree((void*)pte, __FILE__, __LINE__, LIBRARYREQ);
    } else {
        printf("CANNOT FIND PTE WITH INDEX %d for THREAD %d", idx, id);
    }
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
    PTE **pt = (PTE**) mymalloc(len * sizeof(PTE*), __FILE__, __LINE__, LIBRARYREQ);
    memcpy((void*)pt, (void*)SYSINFO->pagetable, PAGETABLE_LEN * sizeof(PTE*));
    SYSINFO->pagetable = pt;

    // init any new threads in the array to NULL
    int i = prev_len;
    for (; i < len; i++) PAGETABLE[i] = NULL;

    myfree((void*) prev_pt, __FILE__, __LINE__, LIBRARYREQ);
}

void add_PTE(int id, int idx, int location) {
    printf("\nADD PTE with idx %d for %d\n", idx, id);
    // make sure thread has an array in our pagetable
    if (id >= PAGETABLE_LEN) {
        resize_pagetable(id + 1);
    }

    // if array is null, init it with size of 1 and add the PTE
    if (!PAGETABLE[id]) {
        PAGETABLE[id] = (PTE*) mymalloc(sizeof(PTE), __FILE__, __LINE__, LIBRARYREQ);
        *PAGETABLE[id] = (PTE) {
            .page_index = idx,
            .page_loc = location,
            .in_swap = 0,
            .next = NULL };
    } else { // add to end
        PTE* cur = PAGETABLE[id];

        // goto last entry
        while (cur->next) cur = cur->next;

        cur->next = (PTE*) mymalloc(sizeof(PTE), __FILE__, __LINE__, LIBRARYREQ);
        *cur->next = (PTE) {
            .page_index = idx,
            .page_loc = location,
            .in_swap = 0,
            .next = NULL };
    }
}

void set_PTE_location(int id, int idx, int location, int in_swap) {
    // handle empty page case
    if (id == -1) {
        return;
    }

    PTE *ptes = PAGETABLE[id];
    PTE *cur = &ptes[0];

    while (cur) {
        if (cur->page_index == idx) {
            cur->page_loc = location;
            cur->in_swap = in_swap;
            break;
        }

        cur = cur->next;
    }
}

/* Page */
int can_access_page(Page * p){
    return (p->id == -1 || p->id == 0 || p->id == get_curr_tcb_id());
}

void swap_pages_swapfile(int mem, int swap) {
    printf("\nSWAP PAGES %d and %d IN SWAPFILE\n", mem, swap);
    if (!can_access_page(&MDATA[mem])) single_chmod(mem, 0);

    int swap_offset = (swap - NUM_PAGES) * PAGE_SIZE;

    // from swap to temp
    fseek(swapfile, swap_offset, SEEK_SET);
    fread(TEMP_PAGE, PAGE_SIZE, 1, swapfile);

    // from mem to swap
    fseek(swapfile, swap_offset, SEEK_SET);
    fwrite(GET_PAGE_ADDRESS(mem), PAGE_SIZE, 1, swapfile);
    
    // from temp to mem
    memcpy(GET_PAGE_ADDRESS(mem), TEMP_PAGE, PAGE_SIZE);

    // update PTE location
    set_PTE_location(MDATA[mem].id, MDATA[mem].idx, swap - NUM_PAGES, 1);
    set_PTE_location(MDATA[swap].id, MDATA[swap].idx, mem, 0);

    // swap mdata
    Page tmp = MDATA[mem];
    MDATA[mem] = MDATA[swap];
    MDATA[swap] = tmp;

    // swap next
    Page * tmpNext = MDATA[mem].next;
    MDATA[mem].next = MDATA[swap].next;
    MDATA[swap].next = tmpNext;
    
    // swap prev
    Page * tmpPrev = MDATA[mem].prev;
    MDATA[mem].prev = MDATA[swap].prev;
    MDATA[swap].prev = tmpPrev;

    // update cur_idx
    int tmpCurIdx = MDATA[mem].cur_idx;
    MDATA[mem].cur_idx = MDATA[swap].cur_idx;
    MDATA[swap].cur_idx = tmpCurIdx;

    // copy front
    MDATA[mem].front = MDATA[swap].front;

    // fix nexts
    fix_entry(&MDATA[mem]);
    
    if (!can_access_page(&MDATA[mem])) single_chmod(mem, 1);
}

void swap_pages(int a, int b) {
    printf("\nSWAP PAGES %d and %d\n", a, b);
    void *a_loc = GET_PAGE_ADDRESS(a);
    void *b_loc = GET_PAGE_ADDRESS(b);

    if (!can_access_page(&MDATA[a])) single_chmod(a,0);
    if (!can_access_page(&MDATA[b])) single_chmod(b,0);

    memcpy(TEMP_PAGE, a_loc, PAGE_SIZE);
    memcpy(a_loc, b_loc, PAGE_SIZE);
    memcpy(b_loc, TEMP_PAGE, PAGE_SIZE);
    
    // update page_loc's
    set_PTE_location(MDATA[a].id, MDATA[a].idx, b, 0);
    set_PTE_location(MDATA[b].id, MDATA[b].idx, a, 0);

    // swap mdata
    Page tmp = MDATA[a];
    MDATA[a] = MDATA[b];
    MDATA[b] = tmp;

    // swap next
    Page * tmpNext = MDATA[a].next;
    MDATA[a].next = MDATA[b].next;
    MDATA[b].next = tmpNext;
    
    // swap prev
    Page * tmpPrev = MDATA[a].prev;
    MDATA[a].prev = MDATA[b].prev;
    MDATA[b].prev = tmpPrev;

    // swap cur_idx
    int tmpCurIdx = MDATA[a].cur_idx;
    MDATA[a].cur_idx = MDATA[b].cur_idx;
    MDATA[b].cur_idx = tmpCurIdx;

    // fix entry next's
    fix_entry(&MDATA[a]);
    fix_entry(&MDATA[b]);

    // swap fronts
    Entry *tmpE = MDATA[a].front;
    MDATA[a].front = MDATA[b].front;
    MDATA[b].front = tmpE;

    if (!can_access_page(&MDATA[a])) single_chmod(a,1);
    if (!can_access_page(&MDATA[b])) single_chmod(b,1);
}

void clear_page(Page * curr){
    printf("\nI AM CLEAR_PAGE\n");
    curr->id = -1;
    curr->is_free = 1;
    curr->mem_free = PAGE_SIZE;
    curr->parent = -1;
    curr->idx = -1;
}

void my_chmod(int id, int protect) {
    if (id >= PAGETABLE_LEN) {
        printf("CHMOD: THREAD#%d NOT IN PAGETABLE YET %d\n", id, PAGETABLE_LEN);
        return;
    }

    //printf("MY_CHMOD: THREAD#%d IS IN PAGETABLE%d\n", id, PAGETABLE_LEN);
    // take the tcb_id and protect flag (1 for change all pages to PROT_NONE, 0 for inverse)
    PTE *cur = PAGETABLE[id];
    while (cur){
        // TODO: check if page is in mem and not swap file
        if (1) {
            single_chmod(cur->page_loc, protect);
        }

        cur = cur->next;
    }
}

void single_chmod(int idx, int protect) {
            protect = 0;
            //printf("%sing page with idx %d for thread #%d\n", protect ? "protect" : "unprotect", idx,  get_curr_tcb_id());
    mprotect(GET_PAGE_ADDRESS(idx), PAGE_SIZE, protect ? PROT_NONE : PROT_READ|PROT_WRITE); 
}

void init_front(Page *p) {
    *p->front = (Entry) {
        .size = MAX_ENTRY_SIZE,
        .next = NULL,
        .is_free = 1 };
}

int is_availible_page(Page *p, int id) {
    printf("\nIS AVAILIBLE: id=%d parent=%d idx=%d cur_idx=%d\n", p->id, p->parent, p->idx, p->cur_idx);
    return (p->id == -1 || p->id == id);
}

int page_is_empty(Page* p) {
    if (!can_access_page(p)) single_chmod(p->idx,0);
    int ret_val = p->front->is_free && !p->front->next;
    if (!can_access_page(p)) single_chmod(p->idx,1);

    return ret_val;
}

int page_not_owned(Page *p) {
    return (p->id == -1);
}

void init_page(Page *p, int id, int parent, int idx) {
    p->id = id;
    p->is_free = 0;
    p->parent = parent;
    p->idx = idx;
}

int find_empty_page(int start){
    printf("\nlooking for empty page\n");

    int i = start;
    for(; i < THREAD_NUM_PAGES; i++){
        Page * p = &MDATA[i];
        if (page_is_empty(p) && p->id == -1){
            return i;
        }
    }

    return -1;
}

int find_empty_swapfile_page(int start) {
    printf("\nlooking for empty swapfile page\n");

    int i = start;
    for (; i < NUM_PAGES + NUM_SWAPFILE_PAGES; i++) {
        Page * p = &MDATA[i];
        if (page_not_owned(p)){
            return i;
        }
    }

    return -1;
}

int is_multipage_malloc(Page *p) {
    if (!can_access_page(p)) single_chmod(p->idx, 0);

    int ret_val = ((!p->front->next) && (!p->front->is_free) && (p->next) && (p->next->parent == p->idx)) || (p->parent != -1 && p->parent != p->idx);

    if (!can_access_page(p)) single_chmod(p->idx, 1);

    return ret_val;
}

/* Entry */
void fix_entry(Page *p){
    Entry *e = p->front;
    while (e->next){
        e->next = (Entry *) ((char *)e + (e->size + sizeof(Entry)));
        e = e->next;
    }
}

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

