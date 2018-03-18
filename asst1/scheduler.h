#include "data_structure.h"
#include <sys/ucontext.h>

// Thread Control Block

typedef struct threadControlBlock {
    int id;
    int p_level;
    ucontext_t context;
    void * retval; // supplied to pthread_exit
} tcb;

tcb *tcb_init();

// Multi Level Queue

typedef struct multiLevelQueue {
    queue ** q_arr;
    int num_levels;
    int interval_time_delta;
    int size;
    int base_time;
} multi_queue;

multi_queue * m_queue_init(int num_levels, int time_delta, int base_time);
void init_job(void * element, multi_queue * m_q);
tcb * get_next_job(multi_queue * m_q);
int is_empty_m_queue(multi_queue * m_q);
int get_interval_time(int level, multi_queue * m_q);
void cleanup_m_queue(multi_queue * m_q);
void bump_old_jobs(double percentage, void * element, multi_queue * m_q);
void add_job(tcb * element, multi_queue * m_q);

// Scheduler

typedef struct scheduler {
    tcb * curr; // current thread
    multi_queue * m_queue; // scheduling queue
    hash_table *terminated;
    hash_table *unlockJobs;
    hash_table *lockOwners;

    // TODO: we currently store the joined job in a min heap inside this
    // hash table. this is because I thought there could be multiple jobs
    // ,but in actuality, only one job should ever join on a particular thread.
    // TL;DR we should insert a tcb instead of min heap
    hash_table *joinJobs;
} sched;

sched *sched_init(int num_queue_levels, int alarm_time_delta,
    int alarm_base_time);
void add_waiting_job(tcb *t, hash_table *jobs, int id);
tcb *remove_waiting_job(hash_table *jobs, int id);
int get_curr_tcb_id();

