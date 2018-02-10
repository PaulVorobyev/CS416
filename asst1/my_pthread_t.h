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
#include <stdio.h>
#include <signal.h>
#include <sys/ucontext.h>
#include "data_structure.h"

/* State and TCB */

typedef uint my_pthread_t;

typedef enum State {
    Running,
    Ready,
    Terminated,
    Waiting,
    Locking
} state_t;

typedef struct threadControlBlock {
    int p_level; //priority level that it's on
    my_pthread_t id;
    ucontext_t context;
    state_t state;
    void * retval; // supplied to pthread_exit
} tcb;

/* Scheduler state struct */
typedef struct scheduler {
    int timerSet; // 0 = false, 1 = true
    int interval; // time in microseconds for alarm to go off
    queue * s_queue; // scheduling queue
    queue * terminated; // TODO merge with s_queue or make this a different ds
    tcb * curr; // current thread
    int mainThreadCreated; // TODO remove this stupid flag
} sched;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
} my_pthread_mutex_t;




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
