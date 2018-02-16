#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

/* Thread Control Block */

// id to be used for next new thread
int nextThreadNumber = 0;

/* initialize a tcb */
tcb * tcb_init() {
    tcb * t = (tcb * ) malloc(sizeof(tcb));
    getcontext(&(t->context));
    t->id = nextThreadNumber++; // TODO: set id = to address for utility
    t->p_level = -1;
    t->state = 0;

    return t;
}

/* Multi Level Queue */

multi_queue * m_queue_init(int num_levels, int time_delta, int base_time){
    // Create queue array
    int i;
    queue ** s_q = (queue **) malloc(num_levels * sizeof(queue*));
    for(i = 0; i < num_levels; i++){
        s_q[i] = queue_init();
    }
    
    // Create multi level q
    multi_queue * q = (multi_queue *) malloc(sizeof(multi_queue));
    q->q_arr = s_q;
    q->num_levels = num_levels;
    q->interval_time_delta = time_delta;
    q->size = 0;
    q->base_time = base_time;

    return q;
}

void add_job(tcb * element, multi_queue * m_q){
    int curr_level = element->p_level;

    printf ("ADD: curr_level: %d\n", curr_level);

    // if on last level, then put on same level
    if(curr_level == m_q->num_levels-1){
        printf("Add same level\n");
        queue_enqueue((void*)element, m_q->q_arr[curr_level]);
    }else if (curr_level < m_q->num_levels-1){
        // add job to next level down
        element->p_level += 1;
        printf("New level: %d\n", element->p_level);
        queue_enqueue((void*)element, m_q->q_arr[curr_level+1]);
    }else{
        printf("ERROR! You're an idiot\n");
    }

    m_q->size += 1;
}

tcb * get_next_job(multi_queue * m_q){
    int i;
    tcb * data = NULL;
    for(i = 0; i < m_q->num_levels; i++){
        queue *q = m_q->q_arr[i];
        if(!isEmpty(q)){
            printf("Grab from queue at level %d\n", i);
            data = (tcb*) queue_dequeue(q);
            m_q->size -= 1;
            break;
        }
    }

    return data;
}

int is_empty_m_queue(multi_queue * m_q){ 
    return m_q->size == 0;
}

int get_interval_time(int level, multi_queue * m_q){
    if(level < 0){
        return m_q->base_time;
    }
    return m_q->base_time + (m_q->interval_time_delta * level);
}

void cleanup_m_queue(multi_queue * m_q){
    int i;
    for(i = 0; i < m_q->num_levels; i++){
        free(m_q->q_arr[i]);
    }

    free(m_q->q_arr);
    free(m_q);
}

/* Scheduler */

sched *sched_init(int num_queue_levels, int alarm_time_delta,
    int alarm_base_time) {

    sched *scheduler = (sched *) malloc(sizeof(sched));
    scheduler->m_queue = m_queue_init(num_queue_levels, alarm_time_delta, alarm_base_time);
    scheduler->terminated = hash_init();
    scheduler->unlockJobs = hash_init();
    scheduler->joinJobs = hash_init();
    scheduler->lockOwners = hash_init();
    
    return scheduler;
}

// in retrospect, we should have used a min heap since a job with a lower
// p_level has a higher priority. To fix this, the comparator returns
// an int > 0 if the p_level of arg1 is lower than arg2
int job_cmp(void *a, void *b) {
    return ((tcb*)b)->p_level - ((tcb*)a)->p_level;
}

void add_waiting_job(tcb *t, hash_table *job_table, int id) {
    m_heap *jobs = (m_heap*) hash_find(job_table, id);

    if (jobs) {
        m_heap_insert(jobs, (void*)t);
        return;
    }

    jobs = m_heap_init(job_cmp);
    m_heap_insert(jobs, (void*)t);
    hash_insert(job_table, (void*)jobs, id);
}

tcb *remove_waiting_job(hash_table *job_table, int id) {
    m_heap *jobs = (m_heap*) hash_find(job_table, id);

    if (jobs) {
        tcb *job = (tcb*) m_heap_delete(jobs);

        // if the heap is now empty, remove the entry from the hashtable
        if (m_heap_is_empty(jobs)) {
            hash_delete(job_table, id);
        }

        return job;
    }

    return NULL;
}

/* Alarm */


