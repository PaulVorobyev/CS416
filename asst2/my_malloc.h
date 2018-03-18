/*
 * File: my_malloc.h
 * Author: Paul Vorobyev (pv149)
 * Date: 03/04/2018
 *
 * Malloc definition
 */

#ifndef MY_MALLOC_H
#define MY_MALLOC_HA

#define _GNU_SOURCE

#include <stdlib.h>

// SYS malloc
#define LIBRARYREQ 1
// // USR malloc
#define THREADREQ 0


//__FILE and __LINE__ are C macros 
#define malloc(x)   mymalloc((x), __FILE__, __LINE__, LIBRARYREQ)
#define free(x)     myfree((x), __FILE__, __LINE__, LIBRARYREQ)

void * mymalloc(size_t size, const char * file, int line, int flag);
void myfree(void * ptr, const char * file, int line, int flag);

#endif
