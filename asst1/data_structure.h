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
#include <sys/ucontext.h>

/* Struct definitions */

// Thread

typedef uint my_pthread_t;

typedef enum State {
    Running,
    Ready,
    Terminated,
    Waiting,
    Locking
} state_t;

typedef struct threadControlBlock {
    my_pthread_t id;
    ucontext_t context;
    state_t state;
} tcb; 

// Queue

typedef struct queueNode {
    void * data;
    struct queueNode * next;
    struct queueNode * prev;
} node;

typedef struct queueStructure {
    node * head;
    node * rear;
    int size;
} queue;

/* Method definitions */

// Queue
queue * queue_init();
void queue_enqueue(void * element, queue * q);
void * queue_dequeue(queue * q);
int isEmpty(queue * q);

#endif
