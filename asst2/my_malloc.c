#include <stdlib.h>
#include <stdio.h>

#include "my_malloc.h"
#include "data_structure.h"
#include "virt_mem.h"

/* Constants */
#define SYSINFO ((SysInfo*)allmem[((char*)(((Entry*)allmem)->next + 1)) + sizeof(SysInfo)])

/* Globals */
static char allmem[ARRAY_SIZE];
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

    // the total number of requested pages
    int req_pages = ceil( (double)size_with_entry / (double)PAGE_SIZE);

    void *data = (flag == LIBRARYREQ) ? sys_malloc() : user_malloc();

    return data;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    return;
}
