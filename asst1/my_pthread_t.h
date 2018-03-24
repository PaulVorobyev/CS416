// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H
#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <sys/types.h>
#include "scheduler.h"

/* Macros */
#define pthread_create(id, attr, fn, arg) my_pthread_create((my_pthread_t*)(id), attr, fn, arg)
#define pthread_yield() my_pthread_yield()
#define pthread_exit(value_ptr) my_pthread_exit(value_ptr)
#define pthread_join(id, value_ptr) my_pthread_join(id, value_ptr)
#define pthread_mutex_init(mutex, attr) my_pthread_mutex_init((my_pthread_mutex_t*)(mutex), attr)
#define pthread_mutex_lock(mutex) my_pthread_mutex_lock((my_pthread_mutex_t*)(mutex))
#define pthread_mutex_unlock(mutex) my_pthread_mutex_unlock((my_pthread_mutex_t*)(mutex))
#define pthread_mutex_destroy(mutex) my_pthread_mutex_destroy((my_pthread_mutex_t*)(mutex))

/* my_pthread_t type definition */
typedef int my_pthread_t;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
    int id;
    int locked; // 0 = unlocked, 1 = locked
} my_pthread_mutex_t;

/* create a new thread */
int my_pthread_create(my_pthread_t *id, const pthread_attr_t *attr, void *(*function)(void*), void *arg);

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

/* Methods to be used in MMU */
void disableAlarm();
void setAlarm();
int get_curr_tcb_id();
int is_sched_init();
int is_in_lib();

#endif
