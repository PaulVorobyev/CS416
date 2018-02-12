#include <sys/ucontext.h>
typedef uint my_pthread_t;

typedef enum State {
    Running,
    Ready,
    Terminated,
    Waiting,
    Locking
} state_t;

typedef struct threadControlBlock {
    my_pthread_t id;
    int p_level;
    ucontext_t context;
    state_t state;
    void * retval; // supplied to pthread_exit
} tcb;
