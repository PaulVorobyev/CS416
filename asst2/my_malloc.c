#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"
#include "data_structure.h"
#include "virt_mem.h"

//no semicolon after #define

/* Constants */

/* Globals */
// Memory array
static char ALLMEM[ARRAY_SIZE];
// page table where index = tcb_id and element = index# of page in ALLMEM
//static int page_table[(ARRAY_SIZE/PAGE_SIZE)];

void * mymalloc(size_t size, const char * file, int line, int flag) {
    printf("Start Malloc\n");
    size_t req_size = size;
    if ((int)req_size <= 0){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line); */
        return 0;
    }

    mem_init(ALLMEM);

    return 0;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    return;
}
