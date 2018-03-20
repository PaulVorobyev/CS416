#include <stdlib.h>
#include <stdio.h>

#include "my_malloc.h"
#include "data_structure.h"
#include "virt_mem.h"

//no semicolon after #define

/* Constants */

/* Globals */
// Memory array
static char allmem[ARRAY_SIZE];
// page table where index = tcb_id and element = index# of page in allmem
static int page_table[(int)(ARRAY_SIZE/PAGE_SIZE)];
Page * page_mdata = NULL;
Page * last_page = NULL;

void * mymalloc(size_t size, const char * file, int line, int flag) {
    printf("Start Malloc\n");
    if ((int)size <= 0){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line); */
        return 0;
    }

    if (!page_mdata){
        last_page = mem_init(allmem);
    }

    // the total number of requested pages
    int req_pages = ceil(size/PAGE_SIZE);
    sys_malloc(allmem, page_table, req_pages);

    if (flag == LIBRARYREQ){
        
    } else {
        
    }

    return 0;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    return;
}
