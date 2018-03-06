#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"
#include "data_structure.h"

//no semicolon after #define
#define ARRAYSIZE 8388608
#define MEMSIZE sizeof(struct MemEntry_)
static char ALLMEM[ARRAYSIZE];
static int initialize = 0;

//function called to add another Mementry if space ran out in char arr
char *expand(MemEntry last, size_t x, char *file, size_t line){
    MemEntry i;
    char *tmp = (char *)sbrk(x + MEMSIZE);
    if(!tmp){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line);*/
        return 0;
    }
    MemEntry ret = (MemEntry)tmp;
    last->next = ret;

    ret->flag = 1;
    ret->size = x;
    ret->next = NULL;
    /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
    return (char *)ret + MEMSIZE;
}



void * mymalloc(size_t size, const char * file, int line, int flag) {
    hash_table * h = hash_init();
    size_t x = size;
    if((int)x <= 0){
        /*fprintf(stderr, "Error! [%s:%d] tried to malloc a negative amount\n", file, line); */
        return 0;
    }
    //printf("\n\nTEST\n");
    MemEntry i; 
    MemEntry root = (MemEntry)ALLMEM;
    int newSize = 0;

    if(!initialize){
        root->flag = 0; //0 means free
        root->size = ARRAYSIZE-MEMSIZE;
        root->next = NULL;
        initialize = 1;
        //printf("peaches\n"); 
    }

    if (root->flag == 0 && root->size+MEMSIZE > x){
        //printf("hi\n"); 
        newSize = root->size-MEMSIZE-x;
        root->size = x;
        root->flag = 1;
        MemEntry newNode =(MemEntry)((char*)root+MEMSIZE+x);
        newNode->size = newSize;
        newNode->flag = 0;
        newNode->next = root->next;
        root->next = newNode;
        //printf("rootsize: %d    nodesize: %d    start: %d   end: %d    retptr: %d\n", root->size, newNode->size, root, (int)(root+1) + x, (int)root+MEMSIZE); 
        /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
        return (char*)root + MEMSIZE;
    }

    //printf("pineapple\n"); 

    if (root->flag == 0 && root->size == x){
        root->size = x;
        root->flag = 1;
        //printf("peanuts\n"); 
        /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
        return (char*)root + MEMSIZE;
    }

    //printf("kiwi\n"); 

    for (i = root; ; i=i->next){
        //printf("kumquat: %d    wanted: %d     size: %d\n", (int)i,x, i->size); 

        if(x > i->size && !i->next && i->flag == 0)
            return expand(i, x, file, line);

        if (i->flag == 0 && i->size+MEMSIZE > x){
            //printf("starfruit\n"); 
            newSize = i->size-MEMSIZE-x;
            i->size = x;
            i->flag = 1;
            MemEntry newNode = (MemEntry)((char*)i+MEMSIZE+x);
            //printf("newsize: %d\n", newSize);
            newNode->size = newSize;
            newNode->flag = 0;
            i->next = newNode;
            newNode->next = NULL;
            //printf("return: %d\n", (int)i+MEMSIZE);
            /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
            return (char*)i + MEMSIZE;
        }

        if (i->flag == 0 && i->size == x){
            //printf("lemons\n"); 
            i->size = x;
            i->flag = 1;
            /*if(strcmp(file, "malloc.c") != 0)printf("Success! [%s:%d] successfully malloced %d bytes\n", file, line, (int)x); */
            return (char*)i + MEMSIZE;
        }
    } 
    return 0;
}

void myfree(void * ptr, const char * file, int line, int flag) {
    MemEntry i, root;   
    root = (MemEntry)ALLMEM;
    //input = (MemEntry)((char*)ptr-MEMSIZE);
    if (!ptr){
        fprintf(stderr, "Error! Ptr does not exist in [%s:%d]\n", file, line); 
        return;
    }
    //if address DOES NOT exist within the memory block
    if (&ptr <= &ALLMEM+MEMSIZE && &ptr >= &ALLMEM+(ARRAYSIZE-MEMSIZE)){
        fprintf(stderr, "Error! Ptr in [%s:%d] is not contained in memory\n", file, line);
        return;
    }
    //if input + ___
    for(i = root; ; i = i->next){
        //printf("i_size: %d\n", i->size); 
        if(i->next == NULL && i+1 != ptr){
            fprintf(stderr, "Error [%s:%d] had an error in freeing\n", file, line);
            return;
        }
        if(i+1 == ptr){
            //printf("guava\n"); 
            break;
        }
    }

    //if ptr address matches with a MemEntry address in array
    if(i->flag == 1){
        i->flag = 0;
    }else{
        fprintf(stderr, "Error! Pointer at [%s:%d] is already free.\n", file, line); 
        return;
    }

    //combining multiple free blocks
    i = root;
    do{
        //printf("flag: %d       before merge: %d      nxt: %d\n",i->flag,  i->size, i->next->size);
        if(i->flag == 0 && i->next->flag == 0){
            //add the size of the next MemEntry to the current and "deletes" the next MemEntry to allow for rewritinig
            i->size += i->next->size + MEMSIZE;
            //if more than 2 MemEntries
            if(i->next->next){
                //printf("more than 2\n");
                i->next = i->next->next;
                i = root; //set i back to root to check thru LL again
                continue;
            }else{
                //printf("less than 2\n"); 
                i->next = NULL;
                break;
            }
        }
        i = i->next;
    }while(i->next);
    //printf("after merge: %d\n", i->size);

    /*printf("Success! [%s:%d] successfully freed\n", file, line); */
    return;
}
