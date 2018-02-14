// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

/* Constants */

// number levels in m_queue
#define NUM_QUEUE_LEVELS 5
// scale factor for time difference between priority levels
#define ALARM_TIME_DELTA 25
// alarm time for highest priority
#define ALARM_BASE_TIME 200

/* Globals */

static sched * scheduler = NULL;
int timesSwitched = 0; // TODO: debug

/* Alarm-related functions */

static void setAlarm() {
    if (scheduler == NULL) {
        puts("Error: scheduler not initialized");
        exit(1);
    }

    int interval = get_interval_time(scheduler->curr->p_level, scheduler->m_queue);

    ualarm(interval, interval);
}

static void disableAlarm() {
    ualarm(0, 0);
}

void alrm_handler(int signo) {
    disableAlarm();

    printf("SWITCH! - %d\n", timesSwitched++); // TODO: debug

    tcb *old = scheduler->curr;
    scheduler->curr =  ((tcb*) get_next_job(scheduler->m_queue));

    // all we got left is main so it should continue running
    if (!scheduler->curr) { 
        scheduler->curr = old;
        return;
    }

    // enqueue old job
    add_job((void*) old, scheduler->m_queue);

    // start next job
    setAlarm();
    swapcontext(&old->context, &scheduler->curr->context);
}

/* initialize a tcb */
tcb * tcb_init() {
    tcb * t = (tcb * ) malloc(sizeof(tcb));
    getcontext(&(t->context));
    t->id = (int) t; // TODO: set id = to address for utility
    t->p_level = -1;
    t->state = 0;

    return t;
}

/* initializes global scheduler variable */
void sched_init() {
    if (scheduler != NULL) {
        printf("Error: scheduler already created!");
        exit(1);
    }

    scheduler = (sched *) malloc(sizeof(sched));
    scheduler->m_queue = m_queue_init(NUM_QUEUE_LEVELS, ALARM_TIME_DELTA, ALARM_BASE_TIME);
    scheduler->terminated = queue_init();
    
    return;
}

/* m_queue */

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
        sched_init();
        signal(SIGALRM, alrm_handler);
    } else {
        old = scheduler->curr;
    }

    // enqueue old thread
    add_job((void *) old, scheduler->m_queue);
    
    // create new thread
    tcb * t = tcb_init();
    getcontext(&(t->context));
    t->context.uc_stack.ss_sp = malloc(MEM);
    t->context.uc_stack.ss_size = MEM;
    t->context.uc_stack.ss_flags = 0;
    t->context.uc_link = &(old->context);
    makecontext(&(t->context), thread_runner, 2, function, arg);

    // start new thread
    scheduler->curr = t;
    setAlarm();
    swapcontext(&(old->context), &(t->context));

	return t->id;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
    disableAlarm();

    puts("YIELD!"); //TODO: debug

    tcb *old = scheduler->curr;
    scheduler->curr = ((tcb*) get_next_job(scheduler->m_queue));

    if (!scheduler->curr) { // all we got left is main
        puts("MAIN IS ONLY THREAD LEFT!"); //TODO: debug

        scheduler->curr = old;
    } else {
        printf("END SWITCH! - %d\n", timesSwitched++); //TODO: debug

        add_job((void*) old, scheduler->m_queue);

        setAlarm();
        swapcontext(&old->context, &scheduler->curr->context);
    }

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
    queue_enqueue(old, &(scheduler->terminated));

    puts("LEAVING EXIT!"); //TODO: debug

    // start next job
    scheduler->curr = (tcb *) get_next_job(scheduler->m_queue);
    setAlarm();
    setcontext(&scheduler->curr->context);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};
