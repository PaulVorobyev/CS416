#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"
#include "data_structure.h"
#include "virt_mem.h"

//no semicolon after #define

/* Constants */

/* Globals */
static mem * memory


//function called to add another Mementry if space ran out in char arr
char *expand(Page last, size_t x, char *file, size_t line){
    Page i;
    char *tmp = (char *)sbrk(x + PAGE_STRUCT_SIZE);
    if(!tmp){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);*/
        return 0;
    }
    Page ret = (Page)tmp;
    last->next = ret;

    ret->is_free = 1;
    ret->size = x;
    ret->next = NULL;
    /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
    return (char *)ret + PAGE_STRUCT_SIZE;
}

void mem_init(){
    printf("Create first memory\n");
    page_table = hash_init();
    root->id = -1;
    root->is_free = 0; //0 means free
    root->size = ARRAY_SIZE-PAGE_STRUCT_SIZE;
    root->next = NULL;
}


void * mymalloc(size_t size, const char * file, int line, int flag) {
    printf("Start Malloc\n");
    size_t req_size = size;
    if((int)req_size <= 0){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line); */
        return 0;
    }
    Page curr_page; 
    Page root = (Page)ALLMEM;
    int newSize = 0;


    if (root->is_free && root->size == req_size){
        root->size = req_size;
        root->is_free = 0;
        /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)req_size); */
        return (char*)root + PAGE_STRUCT_SIZE;
    }


    for (curr_page = root; ; curr_page=curr_page->next){
        //printf("kumquat: %d    wanted: %d     size: %d\n", (int)i,req_size, curr_page->size); 

        if (curr_page->is_free && curr_page->size+PAGE_STRUCT_SIZE > req_size){
            //printf("starfruit\n"); 
            newSize = curr_page->size - PAGE_STRUCT_SIZE - req_size;
            curr_page->size = req_size;
            curr_page->is_free = 0;
            Page newNode = (Page)((char*)curr_page + PAGE_STRUCT_SIZE + req_size);
            //printf("newsize: %d\n", newSize);
            newNode->id = get_curr_tcb_id();
            newNode->size = newSize;
            newNode->is_free = 1;
            curr_page->next = newNode;
            newNode->next = NULL;
            //printf("return: %d\n", (int)i+PAGE_STRUCT_SIZE);
            /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)req_size); */
            return (char*)curr_page + PAGE_STRUCT_SIZE;
        }

        if (curr_page->is_free && curr_page->size == req_size){
            //printf("lemons\n"); 
            curr_page->size = req_size;
            curr_page->is_free = 0;
            /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)req_size); */
            return (char*)curr_page + PAGE_STRUCT_SIZE;
        }
    } 
    return 0;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    Page curr_page, root;   
    root = (Page)ALLMEM;
    //input = (Page)((char*)ptr-PAGE_STRUCT_SIZE);
    if (!ptr){
        fprintf(stderr, "Error! Ptr does not exist in [%s:%d]\n", file, line); 
        return;
    }
    //if address DOES NOT exist within the memory block
    if (&ptr <= &ALLMEM+PAGE_STRUCT_SIZE && &ptr >= &ALLMEM+(ARRAY_SIZE-PAGE_STRUCT_SIZE)){
        fprintf(stderr, "Error! Ptr in [%s:%d] is not contained in memory\n", file, line);
        return;
    }
    //if input + ___
    for(curr_page = root; ; curr_page = curr_page->next){
        //printf("i_size: %d\n", curr_page->size); 
        if(curr_page->next == NULL && curr_page+1 != ptr){
            fprintf(stderr, "Error [%s:%d] had an error in freeing\n", file, line);
            return;
        }
        if(curr_page+1 == ptr){
            //printf("guava\n"); 
            break;
        }
    }

    //if ptr address matches with a Page address in array
    if(!curr_page->is_free){
        curr_page->is_free = 1;
    }else{
        fprintf(stderr, "Error! Pointer at [%s:%d] is already free.\n", file, line); 
        return;
    }

    //combining multiple free blocks
    curr_page = root;
    do{
        //printf("flag: %d       before merge: %d      nxt: %d\n",curr_page->is_free,  curr_page->size, curr_page->next->size);
        if(curr_page->is_free && curr_page->next->is_free){
            //add the size of the next Page to the current and "deletes" the next Page to allow for rewritinig
            curr_page->size += curr_page->next->size + PAGE_STRUCT_SIZE;
            //if more than 2 MemEntries
            if(curr_page->next->next){
                //printf("more than 2\n");
                curr_page->next = curr_page->next->next;
                curr_page = root; //set curr_page back to root to check thru LL again
                continue;
            }else{
                //printf("less than 2\n"); 
                curr_page->next = NULL;
                break;
            }
        }
        curr_page = curr_page->next;
    }while(curr_page->next);
    //printf("after merge: %d\n", curr_page->size);

    /*printf("Success! [%s:%d] successfully freed\n", file, line); */
    return;
}
