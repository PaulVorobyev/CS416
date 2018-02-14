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
#include "general.h"

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

typedef struct multiLevelQueue {
    queue ** q_arr;
    int num_levels;
    int interval_time_delta;
    int size;
    int base_time;
} multi_queue;
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
void * peek(queue * q);
void cycle_next_ele(queue * q);
int isEmpty(queue * q);

// Multi level queue 
multi_queue * m_queue_init(int num_levels, int time_delta, int base_time);
void init_job(void * element, multi_queue * m_q);
void * get_next_job(multi_queue * m_q);
int is_empty_m_queue(multi_queue * m_q);
int get_interval_time(int level, multi_queue * m_q);
void cleanup_m_queue(multi_queue * m_q);


// Hashtable
static int hash_mod(int id, int size);
hash_table * hash_init();
void hash_insert(hash_table * h, void * t, int id);
void * hash_find(hash_table * h, int id);

#endif
