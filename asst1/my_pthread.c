// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Olaolu Emmanuel, Jamie Liao, Paul Vorobyev
// username of iLab: ooe4, jl1806, pv149
// iLab Server: python

#define SCHED
#include "my_pthread_t.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdlib.h>
#include "data_structure.h"

#include "../asst2/my_malloc.h"

/* Constants */

// number levels in m_queue
#define NUM_QUEUE_LEVELS 10
// scale factor for time difference between priority levels
#define ALARM_TIME_DELTA 25
// alarm time for highest priority
#define ALARM_BASE_TIME 200
// bytes to allocate for thread stack
#define MEM 64000
// number of cycles to bump old jobs
#define BUMP_CYCLES 30
// percentage of priority levels to bump
#define PERC_BUMP 0.10

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
            //puts("PI!");
            int old_p_level = scheduler->curr->p_level;
            int highest_p_level = ((tcb*)waiting_jobs->arr[0].data)
                ->p_level;
            // subtract 1 since the add_job() will add 1
            scheduler->curr->p_level = (highest_p_level > -1 )
                ? highest_p_level - 1 : -1;

            if (old_p_level < scheduler->curr->p_level) {
                scheduler->curr->p_level = old_p_level;
            }
        }
    }
}

/* Alarm-related functions */

void setAlarm() {
    if (scheduler == NULL) {
        //puts("Error: scheduler not initialized");
        exit(1);
    }

    int interval = get_interval_time(scheduler->curr->p_level,
        scheduler->m_queue);

    ualarm(interval, interval);
}

void disableAlarm() {
    printf("Disable alarm\n");
    ualarm(0, 0);
}

int get_curr_tcb_id(){
    printf("Set alarm\n");
    return (scheduler && scheduler->curr) ? scheduler->curr->id : -1;
}

int is_sched_init() {
    return (scheduler != NULL && (scheduler->curr));
}

void alrm_handler(int signo) {
    disableAlarm();
    timesSwitched += 1;
    //printf("SWITCH %d\n", timesSwitched);

    //printf("SWITCH! - %d\n", timesSwitched++); // TODO: debug

    // this is only ready thread, let it keep running
    if (is_empty_m_queue(scheduler->m_queue)) {
        CONTINUE_CURRENT_THREAD;
        return;
    }

    priority_inversion_check();

    // bump old jobs if needed
    if (timesSwitched == BUMP_CYCLES){
        bump_old_jobs(PERC_BUMP,
                        scheduler->curr,
                        scheduler->m_queue);
        timesSwitched = 0;
    }

    tcb *old = scheduler->curr;
    tcb *next = get_next_job(scheduler->m_queue);

    add_job((void*) old, scheduler->m_queue);

    SWAP_NEXT_THREAD(old, next);
}

/* Threads */

/* runs a thread function then calls pthread_exit with its ret_val */
void thread_runner(void *(*function)(void*), void *arg) {
    void *ret_val = function(arg);
    my_pthread_exit(ret_val);

    //printf("END THREAD RUNNER: %d", *((int*)ret_val)); // TODO: debug
}

/* create a new thread */
int my_pthread_create(my_pthread_t *id, const pthread_attr_t *attr,
		void *(*function)(void*), void *arg) {
    disableAlarm();

    //puts("CREATE!"); // TODO: debug

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

	*id = t->id;

	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
    // a yield is essentially a premature timer interrupt
    // so we can just call the alarm handler
    alrm_handler(-1);

    return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    disableAlarm();

    //puts("ENTER EXIT!"); //TODO: debug

    // store old thread's ret_val and mark as terminated
    tcb *old = scheduler->curr;
    old->retval = value_ptr;
    hash_insert(scheduler->terminated, (void*)old, old->id);

    tcb *next = remove_waiting_job(scheduler->joinJobs, old->id);
    if (!next) {
        next = get_next_job(scheduler->m_queue);
    }

    // if there are no ready threads and noone is joined on this,
    // then exit the process
    if (!next) {
        exit(0);
    }

    SET_NEXT_THREAD(next);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    disableAlarm();

    tcb *targetThread = (tcb*) hash_find(scheduler->terminated, thread);

    if (targetThread) { // already finished
        if (value_ptr) {
            *value_ptr = targetThread->retval;
        }

        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    tcb *old = scheduler->curr;
    tcb *next = get_next_job(scheduler->m_queue);

    add_waiting_job(old, scheduler->joinJobs, (int)thread);

    if (!next) {
        //puts("Error: joining when you are only thread left"); // TODO: debug

        // just loop infinitely so the user can realize something is wrong
        // and kill the process
        while(1) {};
    }

    SWAP_NEXT_THREAD(old, next);

    // we have been swapped in again because the thread we are joining
    // on is finished
    
    disableAlarm();

    targetThread = (tcb*) hash_find(scheduler->terminated, thread);

    // we messed up. the target thread should be done now
    if (!targetThread) {
        //printf("Error: could not find terminated thread: %d", thread);
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
    tcb *next = get_next_job(scheduler->m_queue);

    add_waiting_job(old, scheduler->unlockJobs, mutex->id);

    if (!next) {
        //puts("Error: waiting on lock, when you are only thread left"); // TODO: debug

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

    //printf("MUTEX%d UNLOCKED!\n", mutex->id + 1);

    mutex->locked = 0;
    hash_delete(scheduler->lockOwners, scheduler->curr->id);

    tcb *next = remove_waiting_job(scheduler->unlockJobs, mutex->id);
    
    // noone is waiting on this lock so continue running
    if (!next) {
        CONTINUE_CURRENT_THREAD;
        return 0;
    }

    tcb *old = scheduler->curr;

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

