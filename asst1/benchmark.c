/*
 * Simulate execution of multiple jobs running on
 * seperate threads using the `my_pthread_t.h` library.
 * Jobs read from a text file containing 2 columns of
 * integers seperated by spaces:
 *
 * col1: number of seconds after the sim starts that the 
 * the thread should be created
 *
 * col2: how many times the job should loop
 *
 * results are printed to stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "my_pthread_t.h"
#include <math.h>

float avg_turnaround_time = 0.0;

typedef struct job {
    int startAfter; // at what time t (seconds) the job started
    int duration; // how many times thread should loop
    unsigned long totalRuntime; // at what time t (microseconds) the job ended
} job;

typedef struct jobArgs {
  int duration;
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

        sscanf(line, "%d %d", &startAfter, &duration);

        job *j = (job*) malloc(sizeof(job));
        j->startAfter = startAfter;
        j->duration = duration;
        j->totalRuntime = 1ul;
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
    const char *border = "-------------------------";
    
    puts("");
    puts(border);

    printf("%s\t%s\t%s\n", "start", "loops", "end");

    for (; i < jobsLen; i++) {
        job *j = jobs[i];
        printf("%d\t%d\t%lu\n", j->startAfter, j->duration, j->totalRuntime);
    }

    puts(border);

    printf("Average Turnaround Time: %f\n", avg_turnaround_time);
}

/*
 * simulation of a job.
 *
 * duration - how many seconds it should run for
 */
void *runJob(void *args) {
    jobArgs *a = (jobArgs*) args;
    int duration = a->duration;

    struct timeval startTime;
    struct timeval endTime;
    gettimeofday(&startTime, NULL);
    int i = 0;
    int j = 67;

    while (i < duration) {
        j = cos(j);
        i++;
    }

    gettimeofday(&endTime, NULL);
    long *runtime = malloc(sizeof(long));
    *runtime = endTime.tv_usec - startTime.tv_usec;

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

            jobArgs *args = (jobArgs*) malloc(sizeof(jobArgs));
            args->duration = jobs[i]->duration;

            pthread_create(&thread, NULL, &runJob, (void*) args);

            threads[i] = thread;
            threadArgs[i] = args;

            i++;
        }
    }

    // join on threads
    i = 0;
    while (i < jobsLen) {
        unsigned long *ret_val;

        my_pthread_join(threads[i], (void**) &ret_val);
        jobs[i]->totalRuntime = *ret_val;
        free(ret_val);
        
        i++;
    }

    // calculate avg_turnaround_time to see how well we did
    i = 0;
    while (i < jobsLen) {
        // run it without threading to see approximately how long it takes
        // to finish without sharing time with other thraeds
        long runtime = *((long*)runJob((void*) threadArgs[i]));

        //printf("THREAD_RUNTIME=%lu NORMAL_RUNTIME=%ld\n", jobs[i]->totalRuntime, runtime);

        avg_turnaround_time += (float) jobs[i]->totalRuntime / runtime;

        i++;
    }
    avg_turnaround_time = (float) avg_turnaround_time / jobsLen;

    i = 0;
    free(threads);
    for (; i < jobsLen; i++) {
        free(threadArgs[i]);
    }
    free(threadArgs);
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
