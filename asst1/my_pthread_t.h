// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

#define MEM 64000

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "data_structure.h"


/**
 * Struct for maintaining shceduler state
 */
typedef struct scheduler {
    int timerSet; // 0 = false, 1 = true
    int interval; // time in microseconds for alarm to go off
    queue * s_queue; // scheduling queue
    tcb * curr; // current thread
    int mainThreadCreated; // TODO remove this stupid flag
} sched;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
} my_pthread_mutex_t;

/* Queue Functions */

queue * queue_init();
void queue_enqueue(void * element, queue * q);
void * queue_dequeue(queue * q);
int isEmpty();


/* Scheduling functions */

/* create a new thread */
int my_pthread_create(void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
