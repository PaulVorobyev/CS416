#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <stdio.h>
#include <unistd.h>

#include "./my_malloc.h"
#include "./my_shalloc.h"
#include "./structures.h"

/* Definitions */
// SYS malloc
#define LIBRARYREQ 1
// // USR malloc
#define THREADREQ 0
// Size of total memory array
#define ARRAY_SIZE (8388608)
#define SWAPFILE_SIZE (ARRAY_SIZE * 2)
#define NUM_SWAPFILE_PAGES ((int)(SWAPFILE_SIZE/PAGE_SIZE))
// Size of Page struct
#define PAGE_STRUCT_SIZE (sizeof(struct Page_))
// Size of the system page itself
#define PAGE_SIZE (sysconf(_SC_PAGE_SIZE))
// total number of pages in memory
#define NUM_PAGES ((int)(ARRAY_SIZE/PAGE_SIZE))
// max size of a mementry
#define MAX_ENTRY_SIZE (PAGE_SIZE - sizeof(Entry))
// SIZE OF MDATA IN PAGE NUMBERS //28-ish
#define MDATA_NUM_PAGES (my_ceil((double)(sizeof(Page) * (NUM_PAGES + NUM_SWAPFILE_PAGES)) / (double)PAGE_SIZE))
// size of mdata
#define MDATA_SIZE (MDATA_NUM_PAGES*PAGE_SIZE)
// proper address for a Page in allmem, given its idx
#define GET_PAGE_ADDRESS(x) ((void *) (allmem + (PAGE_SIZE * x)))
// pointer to SysInfo
#define SYSINFO ((SysInfo*) (allmem + ((NUM_PAGES - MDATA_NUM_PAGES - 1) * PAGE_SIZE) + sizeof(Entry)))
// pointer to pagetable
#define PAGETABLE (SYSINFO->pagetable)
// pointer to mdata
#define MDATA (SYSINFO->mdata)
// number of pages allocated for thread with id x
#define GET_NUM_PTES(x) (((Entry*)(((char*) (&PAGETABLE[x][0])) - sizeof(Entry)))->size / sizeof(PTE))
// get the length of the pagetable
#define PAGETABLE_LEN (((Entry*)PAGETABLE - 1)->size / sizeof(PTE*))
#define SYS_NUM_PAGES (NUM_PAGES - 21)
#define THREAD_NUM_PAGES (NUM_PAGES - SYS_NUM_PAGES)
#define TEMP_PAGE (GET_PAGE_ADDRESS(THREAD_NUM_PAGES))
#define SHALLOC_NUM_PAGES (4)
#define SHALLOC_START_PAGE (THREAD_NUM_PAGES + 1)
#define SYS_PAGE_START (SHALLOC_START_PAGE + SHALLOC_NUM_PAGES)

/* Globals */
extern char *allmem;
extern FILE *swapfile;
extern int is_initialized;
extern int mallocing;

/* Prototypes */
int get_id(int flag);
void print_pagetable();
void print_mem(int flag);
void print_swapfile();
void mem_init();

/* Macros */
#define malloc(x)   mymalloc((x), __FILE__, __LINE__, THREADREQ)
#define free(x)     myfree((x), __FILE__, __LINE__, THREADREQ)
#define shalloc(x)     shalloc(x)

#endif
