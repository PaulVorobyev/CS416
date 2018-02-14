CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a

my_pthread.a: my_pthread.o data_structure.o
	$(AR) libmy_pthread.a my_pthread.o data_structure.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread -g $(CFLAGS) my_pthread.c

data_structure.o: data_structure.h
	$(CC) -pthread $(CFLAGS) data_structure.c

sim: jobsim.c data_structure.o my_pthread.o
	$(CC) -pthread -g -o jobsim data_structure.o my_pthread.o jobsim.c

queue: jobsim.c data_structure.o
	$(CC) -pthread -g -o queue_test data_structure.o queue_test.c

clean:
	rm -rf testfile *.o *.a jobsim jobsim.dSYM queue_test
