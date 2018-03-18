#include <stddef.h>
#include <unistd.h>

// Size of total memory array
#define ARRAY_SIZE 8388608
// Size of Page struct
#define PAGE_STRUCT_SIZE sizeof(struct Page_)
// Size of the system page itself
//#define PAGE_SIZE sysconf(_SC_PAGE_SIZE) --> GIVES ME COMP TIME ERROR SADDAYS
#define PAGE_SIZE 4096

/* Memory */

/*
typedef struct Memory_{
    char allmem[ARRAY_SIZE];
    int page_table[(int)(ARRAY_SIZE/PAGE_SIZE)];
    struct Page_ * last_page;
} Memory;
*/

struct Page_ * mem_init(char ** allmem);
void print_mem(char ** allmem);
int ceil (double num);

/* Pages */

typedef struct Page_{
    int id; // id of TCB; -1 for empty/free page
    //TODO: can later change to check if mem_free == however much a freepage is
    int is_free; // 1 for free; 0 for malloc'd
    size_t mem_free; // the amount of memory that is free inside this page
    struct Page_ * next;
    struct Page_ * prev;
    struct Entry_ * front;
} Page;

// Sub-data structure within a page

typedef struct Entry_{
    size_t size;
    struct Entry_ * next;
} Entry;


Page * create_new_page(int id, int is_free, size_t req_size);