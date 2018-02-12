/*
 * Simulate execution of multiple jobs running on
 * seperate threads using the `my_pthread_t.h` library.
 * Jobs read from a text file containing 3 columns of
 * integers seperated by spaces:
 *
 * col1: number of seconds after the sim starts that the 
 * the thread should be created
 *
 * col2: how many seconds the job should run for
 *
 * col3: number of times thread should yield control
 * (yields will be evenly distributed throughout duration
 * of the job)
 *
 * results are printed to stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "data_structure.h"

void test_m_queue(){
    int i;
    multi_queue * m_q = m_queue_init(3, 10, 50);
    for(i = 0; i < 3; i++){
        int * n = malloc(sizeof(int));
        *n = i;

        tcb * j = malloc(sizeof(tcb));
        j->retval = (int *) n;
        j->p_level = -1;
        add_job(j, m_q);
    }
    
    for(i = 0; i < 10; i++){
        printf("apples--------------\n");
        //print_mq(m_q);
        tcb * n = (tcb * ) get_next_job(m_q);
        printf("poop\n");
        printf("item to dequeue: %d\n", *((int *)(n->retval)));
        add_job(n, m_q);
    }
}


int main(int argc, char* argv[]) {
   printf("START!\n");
    test_m_queue();

}
