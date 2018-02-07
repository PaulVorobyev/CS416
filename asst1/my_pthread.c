// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

/* Globals */
static sched * scheduler = NULL;

/* Queue Functions */

queue * queue_init() {
    queue * q = (queue *) malloc(sizeof(queue));
    q->head = q->rear = NULL;
    q->size = 0;

    return q;
}

void queue_enqueue(void * element, queue * q) {
    node * n = (node *) malloc(sizeof(node));
    n->data = element;
    
    if(isEmpty(q)) {
        // 1 element means rear and head are the same
        q->head = n;
        q->head->next = NULL;
        q->head->prev = NULL;

        q->rear = n;
    } else {
        n->next = NULL;
        n->prev = q->rear;
        q->rear->next = n;
        q->rear = n;
    }

    q->size += 1;
}

void * queue_dequeue(queue * q) {
    if (isEmpty(q)) {
        return NULL;
    } else {
        void * data = q->head->data;

        if (q->head->next == NULL) { // only 1
            q->head = NULL;
        } else { // 2+
            q->head = q->head->next;
            q->head->prev = NULL;
        }

        q->size -= 1;
        
        return data;
    }
}

int isEmpty(queue * q) {
    return q->size == 0;
}

/* Alarm-related functions */

static void setAlarm() {
    if (scheduler == NULL) {
        printf("Error: scheduler not initialized");
        return;
    } else if(scheduler->timerSet == 1) {
        printf("Error: timer already set");
        return;
    }

    scheduler->timerSet = 1;
    ualarm(scheduler->interval, scheduler->interval);
}

/* Scheduling functions */

tcb * tcb_init() {
    tcb * t = (tcb * ) malloc(sizeof(tcb));
    t->id = (int) t;
    t->context = *((ucontext_t *) malloc(sizeof(ucontext_t)));
    t->state = NULL;
    return t;
}

sched * sched_init() {
    if (scheduler != NULL) {
        printf("Scheduler already created!");
        return NULL;
    } else {
        scheduler = (sched *) malloc(sizeof(sched));
        scheduler->timerSet = 0;
        setAlarm();
        scheduler->s_queue = queue_init();
        return scheduler;
    }
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
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
