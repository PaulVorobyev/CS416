// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdlib.h>
#include "data_structure.h"
#include "scheduler.h"

/* Constants */

// number levels in m_queue
#define NUM_QUEUE_LEVELS 5
// scale factor for time difference between priority levels
#define ALARM_TIME_DELTA 25
// alarm time for highest priority
#define ALARM_BASE_TIME 200
// bytes to allocate for thread stack
#define MEM 64000

/* Globals */

static sched * scheduler = NULL;
int nextMutexId = 0; // id of next mutex to be created
int timesSwitched = 0; // TODO: debug

/* Macros */

// start alarm and swap next thread
#define SWAP_NEXT_THREAD(old, next) {\
    scheduler->curr = next;\
    setAlarm();\
    swapcontext(&old->context, &next->context);}\

// start alarm and set next thread
#define SET_NEXT_THREAD(next) {\
    scheduler->curr = next;\
    setAlarm();\
    setcontext(&next->context);}\

// set alarm and continue running current thread
#define CONTINUE_CURRENT_THREAD setAlarm()

/* Priority Inversion Check */

void priority_inversion_check() {
    // handle priority inversion using Original Ceiling Priority Protocol
    // see: https://en.wikipedia.org/wiki/Priority_ceiling_protocol
    my_pthread_mutex_t *mutex =
        (my_pthread_mutex_t*) hash_find(scheduler->lockOwners,
        scheduler->curr->id);
    if (mutex) {
        m_heap *waiting_jobs = (m_heap*) hash_find(scheduler->unlockJobs,
            mutex->id);
        if (waiting_jobs) {
            puts("PI!");
            int highest_p_level = ((tcb*)waiting_jobs->arr[0].data)
                ->p_level;
            // subtract 1 since the add_job() will add 1
            scheduler->curr->p_level = (highest_p_level > -1 )
                ? highest_p_level - 1 : -1;
        }
    }
}

/* Alarm-related functions */

static void setAlarm() {
    if (scheduler == NULL) {
        puts("Error: scheduler not initialized");
        exit(1);
    }

    int interval = get_interval_time(scheduler->curr->p_level,
        scheduler->m_queue);

    ualarm(interval, interval);
}

static void disableAlarm() {
    ualarm(0, 0);
}

void alrm_handler(int signo) {
    disableAlarm();

    printf("SWITCH! - %d\n", timesSwitched++); // TODO: debug

    // this is only ready thread, let it keep running
    if (is_empty_m_queue(scheduler->m_queue)) {
        CONTINUE_CURRENT_THREAD;
        return;
    }

    priority_inversion_check();

    tcb *old = scheduler->curr;
    tcb *next = (tcb*) get_next_job(scheduler->m_queue);

    add_job((void*) old, scheduler->m_queue);

    SWAP_NEXT_THREAD(old, next);
}

/* Threads */

/* runs a thread function then calls pthread_exit with its ret_val */
void thread_runner(void *(*function)(void*), void *arg) {
    void *ret_val = function(arg);
    my_pthread_exit(ret_val);

    printf("END THREAD RUNNER: %d", *((int*)ret_val)); // TODO: debug
}

/* create a new thread */
int my_pthread_create(void *(*function)(void*), void * arg) {
    disableAlarm();

    puts("CREATE!"); // TODO: debug

    tcb * old;
    if (scheduler == NULL) { // should only be executed on first thread create
        old = tcb_init();
        scheduler = sched_init(NUM_QUEUE_LEVELS, ALARM_TIME_DELTA,
            ALARM_BASE_TIME);
        signal(SIGALRM, alrm_handler);
    } else {
        old = scheduler->curr;
    }

    // enqueue old thread
    add_job((void *) old, scheduler->m_queue);
    
    // create new thread
    tcb * t = tcb_init();
    getcontext(&(t->context));
    t->context.uc_link = &(old->context);
    t->context.uc_stack = (stack_t) {.ss_sp = malloc(MEM), .ss_size = MEM,
        .ss_flags = 0};
    makecontext(&(t->context), thread_runner, 2, function, arg);

    SWAP_NEXT_THREAD(old, t);

	return t->id;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
    disableAlarm();

    puts("YIELD!"); //TODO: debug

    // this is only thread ready, let it keep running
    if (is_empty_m_queue(scheduler->m_queue)) {
        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    priority_inversion_check();

    tcb *old = scheduler->curr;
    tcb *next = ((tcb*) get_next_job(scheduler->m_queue));

    add_job((void*) old, scheduler->m_queue);

    SWAP_NEXT_THREAD(old, next);

    return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    disableAlarm();

    puts("ENTER EXIT!"); //TODO: debug

    // store old thread's ret_val and mark as terminated
    tcb *old = scheduler->curr;
    old->retval = value_ptr;
    old->state = Terminated;
    hash_insert(scheduler->terminated, (void*)old, old->id);

    m_heap *joinedJob = (m_heap*) hash_find(scheduler->joinJobs, old->id);
    hash_delete(scheduler->joinJobs, old->id);

    // if there are no ready threads and noone is joined on this,
    // then exit the process
    if (is_empty_m_queue(scheduler->m_queue) && m_heap_is_empty(joinedJob)) {
        exit(0);
    }

    // if there is a job joined on this thread, load that
    // otherwise just load the next one in m_queue
    tcb* next = (joinedJob) ? (tcb*) m_heap_delete(joinedJob)
        : (tcb *) get_next_job(scheduler->m_queue);

    SET_NEXT_THREAD(next);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    disableAlarm();

    tcb *targetThread = (tcb*) hash_find(scheduler->terminated, thread);
    hash_delete(scheduler->terminated, thread);
    if (targetThread) { // already finished
        if (value_ptr) {
            *value_ptr = targetThread->retval;
        }

        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    tcb *old = scheduler->curr;
    tcb *next = (tcb*) get_next_job(scheduler->m_queue);

    add_waiting_job(scheduler, old, scheduler->joinJobs,
        (int)thread);

    if (!next) {
        puts("Error: joining when you are only thread left"); // TODO: debug

        // just loop infinitely so the user can realize something is wrong
        // and kill the process
        while(1) {};
    }

    SWAP_NEXT_THREAD(old, next);

    // we have been swapped in again because the thread we are joining
    // on is finished
    
    disableAlarm();

    targetThread = (tcb*) hash_find(scheduler->terminated, thread);
    hash_delete(scheduler->terminated, thread);

    // we messed up. the target thread should be done now
    if (!targetThread) {
        printf("Error: could not find terminated thread: %d", thread);
        exit(1);
    }

    if (value_ptr) {
        *value_ptr = targetThread->retval;
    }

    CONTINUE_CURRENT_THREAD;
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
    *mutex = (my_pthread_mutex_t) {.id = nextMutexId++, .locked = 0};

	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
    disableAlarm();

    if (!mutex->locked) {
        mutex->locked = 1;
        hash_insert(scheduler->lockOwners, (void*) mutex, scheduler->curr->id);

        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    tcb *old = (tcb*) scheduler->curr;
    tcb *next = (tcb*) get_next_job(scheduler->m_queue);

    add_waiting_job(scheduler, old, scheduler->unlockJobs, mutex->id);

    if (!next) {
        puts("Error: waiting on lock, when you are only thread left"); // TODO: debug

        // just loop infinitely so the user can realize something is wrong
        // and kill the process
        while(1) {};
    }

    SWAP_NEXT_THREAD(old, next);
    
    // we have been swapped in again because the mutex is now unlocked
    
    disableAlarm();
    
    mutex->locked = 1;
    hash_insert(scheduler->lockOwners, (void*) mutex, scheduler->curr->id);

    CONTINUE_CURRENT_THREAD;
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    disableAlarm();

    printf("MUTEX%d UNLOCKED!\n", mutex->id + 1);

    mutex->locked = 0;
    hash_delete(scheduler->lockOwners, scheduler->curr->id);

    m_heap *waiting_jobs = hash_find(scheduler->unlockJobs, mutex->id);
    
    // noone is waiting on this lock so continue running
    if (!waiting_jobs) {
        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    tcb *old = scheduler->curr;
    tcb *next = (tcb*) m_heap_delete(waiting_jobs);

    // we've emptied it so it should be removed from table
    if (m_heap_is_empty(waiting_jobs)) {
        hash_delete(scheduler->unlockJobs, mutex->id);
    }

    add_job((void*) old, scheduler->m_queue);

    SWAP_NEXT_THREAD(old, next);

	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
    disableAlarm();
    CONTINUE_CURRENT_THREAD;
	return 0;
};

