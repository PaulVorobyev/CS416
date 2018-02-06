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
#include "my_pthread_t.h"

typedef struct job {
    int startAfter;
    int duration;
    int numIOs;
    int totalRuntime;
} job;

typedef struct jobArgs {
  int duration;
  int numIOs;
} jobArgs;

int jobCmp(const void *a, const void *b) {
    return ((job*) *((job**) a))->startAfter - ((job*) *((job**) b))->startAfter;
}

/*
 * parses input txt files and populates job array
 *
 * job - ptr to an array of job pointers. populates this
 * array with jobs
 *
 * return - len of job array
 */
int parseJobList(const char *filename, job ***jobsPtr) {
    FILE *fp = fopen(filename, "r");
    char *line = NULL;
    size_t size = 0;
    *jobsPtr = (job**) malloc(sizeof(job*) * 100);
    job **jobs = *jobsPtr;
    int jobsLen = 0;

    while (getline(&line, &size, fp) != -1) {
        int startAfter;
        int duration;
        int numIOs;

        sscanf(line, "%d %d %d", &startAfter, &duration, &numIOs);

        job *j = (job*) malloc(sizeof(job));
        j->startAfter = startAfter;
        j->duration = duration;
        j->numIOs = numIOs;
        j->totalRuntime = 10;
        jobs[jobsLen] = j;
        jobsLen++;

        free(line);
        line = NULL;
    } 

    fclose(fp);

    return jobsLen;
}

void printBenchmarks(job **jobs, int jobsLen) {
    int i = 0;

    printf("%s\t%s\t%s\t%s\t%s\n", "no.", "startAfter",
        "dur", "numIOs", "runtime");

    for (; i < jobsLen; i++) {
        job *j = jobs[i];
        printf("%d.\t%d\t\t%d\t%d\t\t%d\n", i, j->startAfter,
            j->duration, j->numIOs, j->totalRuntime);
    }
}

/*
 * simulation of a job.
 *
 * duration - how many seconds it should run for
 * numIOs - number of times it should yield
 */
void *runJob(int duration, int numIOs) {
    struct timeval startTime;
    struct timeval endTime;
    struct timeval curTime;
    gettimeofday(&startTime, NULL);
    double ioInterval = (double) duration / (double) numIOs;

    while (1) {
        gettimeofday(&curTime, NULL);
        int curDuration = curTime.tv_sec - startTime.tv_sec;
        if (curDuration > duration) {
            break;
        } else if (curDuration > ioInterval) {
          //my_pthread_yield();
          ioInterval += ioInterval;
        }

        sleep(1);
    }

    gettimeofday(&endTime, NULL);
    long int *runtime = malloc(sizeof(long int));
    *runtime = startTime.tv_sec - endTime.tv_sec;

    return (void*) runtime;
}

/*
 * free memory from main
 */
void cleanup(job **jobs, int jobsLen) {
    int i = 0;
    
    for(; i < jobsLen; i++) {
        free(jobs[i]);
    }
    free(jobs);
}

/*
 * runs jobs and updates job structs with
 * runtime stats
 */
void runSim(job **jobs, int jobsLen) {
    // sort jobs by start time
    qsort((void*)jobs, jobsLen, sizeof(job*), jobCmp);

    my_pthread_t *threads = (my_pthread_t*) malloc(sizeof(my_pthread_t) * jobsLen);
    jobArgs **threadArgs = (jobArgs**) malloc(sizeof(jobArgs*) * jobsLen);
    struct timeval simStartTime;
    struct timeval simCurTime;
    int i;

    gettimeofday(&simStartTime, NULL);

    // create threads
    i = 0;
    while (i < jobsLen) {
        gettimeofday(&simCurTime, NULL);
        int passedTimeToLaunch = (simCurTime.tv_sec - simStartTime.tv_sec) >= jobs[i]->startAfter;
        if (passedTimeToLaunch) {
            my_pthread_t thread;
            pthread_attr_t *attr = NULL;

            jobArgs *args = (jobArgs*) malloc(sizeof(jobArgs));
            args->duration = jobs[i]->duration;
            args->numIOs = jobs[i]->numIOs;

            //my_pthread_create(&thread, attr, runJob, (void*) args);

            threads[i] = thread;
            threadArgs[i] = args;

            i++;
        }

        sleep(1);
    }

    // join on threads
    i = 0;
    while (i < jobsLen) {
        long int *ret_val;

        //my_pthread_join(threads[i], (void**) &ret_val);
        //jobs[i]->totalRuntime = *ret_val;
        free(ret_val);
        

        i++;
    }

    i = 0;
    free(threads);
    for (; i < jobsLen; i++) {
        free(threadArgs[i]);
    }
    free(threadArgs);
}

void test_queue() {
    queue * q = queue_init();
    for (int i = 0; i < 10; i++) {
        int * j = malloc(sizeof(int));
        *j = i;
        queue_enqueue(j, q);
    }
    
    for (int i = 0; i < 10; i++) {
        printf("%d\n", *((int *) queue_dequeue(q)));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("Usage: ./jobsim <jobs.txt>\n");
        return EXIT_FAILURE;
    }

    int jobsLen = 0;
    job **jobs = NULL;
    jobsLen = parseJobList(argv[1], &jobs);

    runSim(jobs, jobsLen);
    printBenchmarks(jobs, jobsLen);
    cleanup(jobs, jobsLen);

    return EXIT_SUCCESS;
}
