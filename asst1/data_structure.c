#include "data_structure.h"

/* Single Queue Functions */

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
void add_job(void * element, multi_queue * m_q){
    int curr_level = ((tcb *)element)->p_level;

    printf ("curr_level: %d\n", curr_level);

    // if on last level, then put on same level
    if(curr_level == m_q->num_levels-1){
        printf("Add same level\n");
        queue_enqueue(element, m_q->q_arr[curr_level]);
    }else if (curr_level < m_q->num_levels-1){
        // add job to next level down
        printf("Add next level\n");
        queue_enqueue(element, m_q->q_arr[curr_level+1]);
        ((tcb *)element)->p_level += 1;
    }else{
        printf("ERROR! You're an idiot\n");
    }

    m_q->size += 1;
}

void * get_next_job(multi_queue * m_q){
    if(is_empty_m_queue(m_q)){
        return 0;
    }

    int i;
    queue * q;
    void * data;
    for(i = 0; i < m_q->num_levels; i++){
        q = m_q->q_arr[i];
        if(!isEmpty(q)){
            printf("Grab from queue at level %d\n", i);
            data = queue_dequeue(q);
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


