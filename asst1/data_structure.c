#include "data_structure.h"

/* Single Queue Functions */

queue * queue_init() {
    queue * q = (queue *) malloc(sizeof(queue));
    q->head = q->rear = NULL;
    q->size = 0;

    return q;
}

void queue_enqueue(void * element, queue * q) {
    /*
     * @params:
     *      - element (assume always node *)
     *      - q (single queue)
     */
    node * n = (node *) element;
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

// return node
void * queue_dequeue(queue * q) {
    /*
     * @params: a single queue (queue *)
     *
     * @return:
     *      the first node in queue
     */
    if (isEmpty(q)) {
        puts("I HAVE NOTHING!");
        return NULL;
    } else {
        void * data = q->head;

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

void * peek(queue * q){
    if (isEmpty(q)) {
        puts("I HAVE NOTHING!");
        return NULL;
    } else {
        return q->head;
    }
}

int isEmpty(queue * q) {
    return q->size == 0;
}

/* Multi Level Queue Functions */

multi_queue * m_queue_init(int num_levels, int time_delta, int base_time){
    // Create queue array
    int i;
    queue ** s_q = (queue *) malloc(num_levels * sizeof(queue));
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

// Assume element is always TCB
void init_job(void * element, multi_queue * m_q){
    node * n = (node *) malloc(sizeof(node));
    n->data = element;
    n->state = Ready;
    n->p_level = -1;

    add_job(n, m_q);
}

// Assume element is always NODE
void add_job(node * element, multi_queue * m_q){
    int curr_level = element->p_level;

    // if on last level or if waiting on mutex, then put on same level
    if(curr_level == m_q->num_levels-1 || element->state == Waiting){
        queue_enqueue(element, m_q->q_arr[curr_level]);
    }else{
        // add job to next level down
        queue_enqueue(element, m_q->q_arr[curr_level+1]);
    }

    m_q->size += 1;
}

void * get_next_job(multi_queue * m_q){
    if(is_empty_m_queue(m_q)){
        return 0;
    }

    int i;
    queue * q;
    node * n;
    for(i = 0; i < m_q->num_levels; i++){
        q = m_q->q_arr[i];
        if(!isEmpty(q)){
            n = queue_dequeue(q);
            m_q->size -= 1;
        }
    }
    return n;
}

int is_empty_m_queue(multi_queue * m_q){ 
    return m_q->size == 0;
}

int get_interval_time(int level, multi_queue * m_q){
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


