#include "data_structure.h"

/* Single Queue Functions */

queue * queue_init() {
    queue * q = (queue *) malloc(sizeof(queue));
    q->head = q->rear = NULL;
    q->size = 0;

    return q;
}

void queue_enqueue(void * element, queue * q) {
    
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
        puts("I HAVE NOTHING!");
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

/* Multi Level Queue Functions */

multi_queue * m_queue_init(int num_levels, int time_delta, int base_time){
    multi_queue * q = (multi_queue *) malloc(sizeof(multi_queue));
    q->q_arr = (queue *) malloc(sizeof(queue*num_levels));
    q->interval_time_delta = time_delta;
    q->base_time = base_time;

    return q;
}

void init_job(void * element, multi_queue * m_q){
    node * n = (node *) malloc(sizeof(node));
    n->data = element;
    n->state = Ready;

    add_job(n, m_q->q_arr);
}

// Assume element is always NODE
void add_job(void * element, multi_queue * m_q){
    if(element->data->p_level == m_q->num_levels-1){
        // if on last level
        queue_enqueue(element, q_arr[element->data->p_level]);
    }else if{
        // if waiting on mutex, put back on same level
        queue_enqueue(element, q_arr[element->data->p_level]);
    }else{
        // add job to next level down
        queue_enqueue(element, q_arr[element->data->p_level+1]);
    }
}

void * get_next_job(){
    return 0;
}

int is_empty_mqueue(){ 
    return 0;

}

int get_interval_time(int level, multi_queue * m_q){
    return m_q->base_time + (m_q->interval_time_delta * level);
}

void cleanup_m_queue(multi_queue * q){
    free(q->queue_arr);
    free(q);

}


