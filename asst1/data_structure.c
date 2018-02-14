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

/* Hashtable method implementations */

hash_table * hash_init() {
    hash_table * h = (hash_table *) malloc(sizeof(hash_table));
    h->size = 100;
    h->elements = malloc(sizeof(queue *) * (h->size + 1));
    h->hash = &hash_mod;

    // initialize all queue chains
    int i = 0;
    while (i < h->size) {
        h->elements[i] = queue_init();
        i++;
    }
    return h;
}

static int hash_mod(int id, int size) {
    return id % size;
}

void hash_insert(hash_table * h, void * t, int id) {
    int idx = h->hash(id, h->size);
    queue * q  = h->elements[idx];
    entry * e = (entry *) malloc(sizeof(entry));
    e->data = t;
    e->id = id;
    queue_enqueue(e, q);
    return;
}

void * hash_find(hash_table *h, int id) {
    int idx = h->hash(id, h->size);
    node * n  = h->elements[idx]->head;
    entry * e = (entry *) n->data;
    void * t = NULL;
    while (n != NULL) {
        if (e->id == id) {
            t = e->data;
            break;
        }
        n = n->next;
    }
    return t;
}
