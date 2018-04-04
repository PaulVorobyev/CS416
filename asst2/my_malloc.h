/*
 * File: my_malloc.h
 * Author: Paul Vorobyev (pv149)
 * Date: 03/04/2018
 *
 * Malloc definition
 */

#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#define _GNU_SOURCE

#include <stdlib.h>

void * mymalloc(size_t size, const char * file, int line, int flag);
void myfree(void * ptr, const char * file, int line, int flag);

#endif
