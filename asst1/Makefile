CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

Target: my_pthread.a

my_pthread.a: my_pthread.o data_structure.o m_queue.o
	$(AR) libmy_pthread.a my_pthread.o data_structure.o m_queue.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread -g $(CFLAGS) my_pthread.c

data_structure.o: data_structure.h
	$(CC) $(CFLAGS) data_structure.c

m_queue.o: m_queue.h
	$(CC) $(CFLAGS) m_queue.c

sim: jobsim.c data_structure.o my_pthread.o m_queue.o
	$(CC) -pthread -g -o jobsim data_structure.o my_pthread.o m_queue.o jobsim.c

clean:
	rm -rf testfile *.o *.a jobsim jobsim.dSYM queue_test
