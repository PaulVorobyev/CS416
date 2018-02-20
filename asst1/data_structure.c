#include "data_structure.h"
#include <stdlib.h>
#include <stdio.h>

/* Queue Implementation */

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
        //puts("I HAVE NOTHING!");
        return NULL;
    } else {
        node * n = q->head;
        void * data = n->data;

        if (q->head->next == NULL) { // only 1
            q->head = NULL;
        } else { // 2+
            q->head = q->head->next;
            q->head->prev = NULL;
        }

        free(n);
        q->size -= 1;

        return data;
    }
}

void * peek(queue * q){
    if (isEmpty(q)) {
        //puts("I HAVE NOTHING!");
        return NULL;
    } else {
        return q->head;
    }
}

int isEmpty(queue * q) {
    return q->size == 0;
}

/* Hashtable method implementations */

typedef struct hash_table_entry {
    void * data;
    int id;
} entry;

static int hash_mod(int id, int size) {
    return id % size;
}

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

void free_hash(hash_table * h){
    free_queue_arr(elements);
    free(h);
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
    void * t = NULL;
    while (n != NULL) {
        entry * e = (entry *) n->data;
        if (e->id == id) {
            t = e->data;
            break;
        }
        n = n->next;
    }
    return t;
}

void hash_delete(hash_table *h, int id) {
    int idx = h->hash(id, h->size);
    node * n  = h->elements[idx]->head;
    while (n != NULL) {
        entry * e = (entry *) n->data;
        if (e->id == id) {
            queue *q = h->elements[idx];

            if (n->prev) {
                n->prev->next = n->next;
            } else {
                q->head = n->next;
            }

            q->size -= 1;

            break;
        }
        n = n->next;
    }
}

/* Min Heap Method implementations */

#define HEAP_BASE_CAPACITY 10

void m_heap_swap(m_heap *h, int a, int b) {
    void *tmp = h->arr[a].data;
    h->arr[a].data = h->arr[b].data;
    h->arr[b].data = tmp;
}

h_node *m_heap_resize(m_heap *h) {
    int new_capacity = (h->capacity * 2) * sizeof(h_node);
    h_node *new_arr = (h_node*) realloc((void*) h->arr, new_capacity);
    h->capacity *= 2;

    return new_arr;
}

void m_heap_sift_down(m_heap *h, int idx) {
    if (idx >= h->size) {
        return;
    }

    int left_child_idx = (2*idx) + 1;
    int right_child_idx = (2*idx) + 2;

    int no_children = (left_child_idx >= h->size)
        && (right_child_idx >= h->size);
    if (no_children) {
        return;
    }

    // determine index of smaller child
    int smaller_child_idx = -1;
    if (right_child_idx >= h->size) { // only a left child
        smaller_child_idx = left_child_idx;
    } else {
        smaller_child_idx = (h->cmp(h->arr[left_child_idx].data,
            h->arr[right_child_idx].data) < 0)
            ? left_child_idx : right_child_idx;
    }

    // if node is greater than child, swap values and continue
    // sift down on new child
    if ((h->cmp(h->arr[idx].data, h->arr[smaller_child_idx].data) > 0)) {
        m_heap_swap(h, idx, smaller_child_idx);
        m_heap_sift_down(h, smaller_child_idx);
    }
}

void m_heap_sift_up(m_heap *h, int idx) {
    if (idx < 0) {
        return;
    }

    int parent_idx = (idx - 1) / 2;

    // if node is less than its parent, swap values and continue
    // sift up on new parent
    if ((parent_idx >= 0) &&
        (h->cmp(h->arr[idx].data, h->arr[parent_idx].data) < 0)) {
        m_heap_swap(h, idx, parent_idx);
        m_heap_sift_up(h, parent_idx);
    }
}

m_heap *m_heap_init(cmp_fn cmp) {
    m_heap *h = (m_heap*) malloc(sizeof(m_heap));
    *h = (m_heap) {
        .cmp = cmp,
        .arr = (h_node*) malloc(sizeof(h_node) * HEAP_BASE_CAPACITY),
        .size = 0,
        .capacity = HEAP_BASE_CAPACITY
    };

    return h;
}

int m_heap_is_empty(m_heap *h) {
    return h->size == 0;
}

void m_heap_insert(m_heap *h, void *data) {
    if (h->size == h->capacity) {
        h->arr = m_heap_resize(h);
    }

    h->arr[h->size] = (h_node) {.data = data};
    m_heap_sift_up(h, h->size);
    h->size += 1;
}

void *m_heap_delete(m_heap *h) {
    if (m_heap_is_empty(h)) {
        return NULL;
    }

    void *data = h->arr[0].data;

    // copy last node's data to root and delete last node
    h->arr[0].data = h->arr[h->size - 1].data;
    h->size -= 1;
    
    // sift down new root
    m_heap_sift_down(h, 0);

    return data;
}

