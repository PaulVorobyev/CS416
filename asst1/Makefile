CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a

my_pthread.a: my_pthread.o
	$(AR) libmy_pthread.a my_pthread.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread $(CFLAGS) my_pthread.c

sim: jobsim.c my_pthread.o
	$(CC) -g -o jobsim my_pthread.o jobsim.c 

clean:
	rm -rf testfile *.o *.a jobsim
