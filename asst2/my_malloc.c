#include <stdlib.h>
#include <stdio.h>

#include "my_malloc.h"
#include "virt_mem.h"

/* Globals */
int is_initialized = 0;

void * mymalloc(size_t size, const char * file, int line, int flag) {
    printf("Start Malloc\n");
    if ((int)size <= 0){
        // fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);
        return NULL;
    }

    if (!is_initialized) {
        mem_init(allmem);
        is_initialized = 1;
    }

    int size_with_entry = size + sizeof(Entry);

    // if its a libraryreq then its sys i.e. 0
    // if not, then check what scheduler's curr is.
    // if thats not -1, then use it, but if it is
    // then it must be the main thread making the request
    // and its id is (will be) 1
    int current_thread = -1; // TODO: get cur function
    int id = (flag == LIBRARYREQ) ? 0 : 
        (current_thread != -1) ? current_thread : 1; 

    // the total number of requested pages
    int req_pages = ceil((double)size_with_entry / (double)PAGE_SIZE);

    void *data = _malloc(req_pages, size_with_entry, id);

    return data;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    /*int i = 0;
    int current_thread = 0; // get cur function

    // if its a libraryreq then its sys i.e. 0
    // if not, then check what scheduler's curr is.
    // if thats not -1, then use it, but if it is
    // then it must be the main thread making the request
    // and its id is (will be) 1
    int id = (flag == LIBRARYREQ) ? 0 : 
        (current_thread != -1) ? current_thread : 1; 

    for (; i < GET_NUM_PTES(id); i++) {
        Entry *e = PAGETABLE[id][i].front;

        while (e) {
            if (((void*)(e + 1)) == ptr) {
                if (e->is_free) {
                    // complain thats its alrady free?
                    
                    return;
                }

                e->is_free = 1;
                // TODO: coalesce
                
                return;
            }

            e = e->next;
        }
    }

    // complain because we cant find it?

    return;*/
}

