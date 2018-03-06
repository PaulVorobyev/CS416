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

struct MemEntry_{
  int flag;
  size_t size;
  struct MemEntry_* next;
};

typedef struct MemEntry_* MemEntry;

//__FILE and __LINE__ are C macros 
#define malloc(x)   mymalloc((x), __FILE__, __LINE__)
#define free(x)     myfree((x), __FILE__, __LINE__)

char *expand(MemEntry last, size_t x, char *file, size_t line);

void * mymalloc(size_t size, const char * file, int line, int flag);
void myfree(void * ptr, const char * file, int line, int flag);

#endif
