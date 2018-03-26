/*
 * Unit tests
 */

#include <stdio.h>
#include <stdlib.h>
#include "my_pthread_t.h"
#include "../common/data_structure.h"

#include "../asst2/my_malloc.h"

/* Globals */

my_pthread_mutex_t mutex1;
my_pthread_mutex_t mutex2;

/* Test Jobs */

void *foo() {
    int i = 0;
    for (i = 0; i < 2000; i++) {
        //printf("foo! %d\n", i);
    }

    int * result = (int*) malloc(sizeof(int));
    *result = 1;
    return (void*) result;
}

void *bar() {
    int i = 0;
    for (i = 0; i < 2000; i++) {
        printf("bar! %d\n", i);
    }

    int * result = (int*) malloc(sizeof(int));
    *result = 2;
    return (void*) result;
}

void *take_lock1() {
    pthread_mutex_lock(&mutex1);
    puts("TL1 LOCKED MUTEX1!");
    
    pthread_yield();

    pthread_mutex_unlock(&mutex1);

    puts("TL1 ATTEMPTS TO LOCK MUTEX2!");
    pthread_mutex_lock(&mutex2);
    puts("TL1 LOCKED MUTEX2!");

    pthread_mutex_unlock(&mutex2);

    return NULL;
}

void *p1() {
    pthread_mutex_lock(&mutex1);
    pthread_yield();
    pthread_yield();
    pthread_yield();
    pthread_yield();
    pthread_yield();
    pthread_yield();
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void *p2() {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void *take_lock2() {
    puts("TL2 ATTEMPTS TO LOCK MUTEX1!");
    pthread_mutex_lock(&mutex1);
    puts("TL2 LOCKED MUTEX1!");

    pthread_mutex_lock(&mutex2);
    puts("TL2 LOCKED MUTEX2!");

    pthread_yield();

    pthread_mutex_unlock(&mutex1);

    pthread_mutex_unlock(&mutex2);

    return NULL;
}

/* Tests */

void test_queue() {
    puts("\nSTART TEST QUEUE");

    queue * q = queue_init();
    int i = 0;
    for (i = 0; i < 10; i++) {
        int * j = malloc(sizeof(int));
        *j = i;
        queue_enqueue(j, q);
    }
    
    for (i = 0; i < 10; i++) {
        printf("%d ", *((int *) queue_dequeue(q)));
    }
    puts("");

    puts("END TEST QUEUE");
}

void test_hash() {
    puts("\nSTART TEST HASH");

    hash_table * h = hash_init();
    int i = 0;
    for (i = 0; i < 10; i++) {
        int * j = malloc(sizeof(int));
        *j = i;
        hash_insert(h, j, *j);
    }
    
    for (i = 0; i < 10; i++) {
        printf("%d ", *((int *) hash_find(h, i)));
    }
    puts("");

    puts("END TEST HASH");
}

void test_m_queue(){
    puts("\nSTART TEST M_QUEUE");

    puts("SPAWN TWO THREADS");
	my_pthread_t t;
    pthread_create(&t, NULL, &foo, NULL);
    pthread_create(&t, NULL, &bar, NULL);
   
    int i = 0;
    for(; i < 2000; i++){
        printf("Main %d\n", i);
    }

    /*
    for (; i < 5; i++) {
        pthread_yield();
    }
    */

    puts("END TEST M_QUEUE");
}

int int_cmp(void *a, void *b) {
    return *((int*)a) - *((int*)b);
}

void test_m_heap(){
    puts("\nSTART TEST M_HEAP");

    m_heap *h = m_heap_init(int_cmp);
    
    int i = 0;
    for (; i < 20; i++) {
        int *j = (int*) malloc(sizeof(int));
        *j = i;
        m_heap_insert(h, (void*)j);
    }

    // mix ups
    int nums[] = {50, 6, 25, 7};
    m_heap_insert(h, (void*)(&nums[0]));
    m_heap_insert(h, (void*)(&nums[1]));
    m_heap_insert(h, (void*)(&nums[2]));
    m_heap_insert(h, (void*)(&nums[3]));

    i = 0;
    for (; i < 24; i++) {
        printf("%d ", *((int*)m_heap_delete(h)));
    }
    puts("");

    puts("END TEST M_HEAP");
}

void test_join(){
    puts("\nSTART TEST JOIN");

    puts("SPAWN THREE THREADS");
	my_pthread_t f, b, b2;
    pthread_create(&f, NULL, &foo, NULL);
    pthread_create(&b, NULL, &bar, NULL);
	pthread_create(&b2, NULL, &bar, NULL);

    void *f_ret = NULL;
    void *b_ret = NULL;
    void *b2_ret = NULL;
    pthread_join(f, &f_ret);
    pthread_join(b, &b_ret);
    pthread_join(b2, &b2_ret);

    /* printf("Threads returned: %d, %d, %d\n", (*(int*)f_ret), */
    /*     (*(int*)b_ret), (*(int*)b2_ret)); */

    puts("END TEST JOIN");
}

void test_mutex(){
    puts("\nSTART TEST MUTEX");

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

	my_pthread_t t1, t2;
    pthread_create(&t1, NULL, &take_lock1, NULL);
    pthread_create(&t2, NULL, &take_lock2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    puts("END TEST MUTEX");
}

void test_priority_inversion(){
    puts("\nSTART TEST PRIORITY INVERSION");

	my_pthread_t pt1;
    pthread_create(&pt1, NULL, &p1, NULL);
    pthread_yield();
    pthread_yield();
    pthread_yield();
	my_pthread_t pt2;
    pthread_create(&pt2, NULL, &p2, NULL);

    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    puts("END TEST PRIORITY INVERSION");
}

void test_malloc_basic() {
    puts("\nSTART TEST BASIC MALLOC");
    
    int i;
    void *a = malloc(10);
    void *b = malloc(10);
    void *c = malloc(10);
    void *d = malloc(10);
    free(a);
    free(b);
    free(c);
    free(d);

    void *e = malloc(5500);
    //free(e);

    puts("END TEST BASIC MALLOC");
}

int main(int argc, char* argv[]) {
	my_pthread_t t;
    pthread_create(&t, NULL, &foo, NULL);
    pthread_join(t, NULL);
    test_m_queue();
    test_hash();
    test_queue();
    test_m_heap();
    test_join();
    test_mutex();
    test_priority_inversion();

    test_malloc_basic();
    printf("EXIT\n");

    printf("BEFORE 9\n");

    malloc(9);

    shalloc(100);
    shalloc(100);
    shalloc(5500);

    printf("AFTER 9\n");

}
