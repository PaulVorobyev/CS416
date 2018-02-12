/*
 * File: data_structure.h
 * Author: Paul Vorobyev (pv149)
 * Date: 02/08/2018
 *
 * All primitive data structures and associated methods used 
 * in the scheduler are defined here.
 */

#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>

/* Struct definitions */

// Queue

typedef struct linked_node {
    void * data;
    struct linked_node * next;
    struct linked_node * prev;
} node;

typedef struct queue_structure {
    node * head;
    node * rear;
    int size;
} queue;

// Hashtable

typedef int (*hash_fn)(int, int); // hashes id
typedef struct hash_table_entry {
    void * data;
    int id;
} entry;
typedef struct hash_table_structure {
    queue ** elements;
    int size;
    hash_fn hash;
} hash_table;

/* Method definitions */

// Queue
queue * queue_init();
void queue_enqueue(void * element, queue * q);
void * queue_dequeue(queue * q);
int isEmpty(queue * q);

// Hashtable
static int hash_mod(int id, int size);
hash_table * hash_init();
void hash_insert(hash_table * h, void * t, int id);
void * hash_find(hash_table * h, int id);

#endif
