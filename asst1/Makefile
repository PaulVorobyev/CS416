CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

Target: my_pthread.a

my_pthread.a: my_pthread.o data_structure.o scheduler.o
	$(AR) libmy_pthread.a my_pthread.o data_structure.o scheduler.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread -g $(CFLAGS) my_pthread.c

data_structure.o: data_structure.h
	$(CC) $(CFLAGS) data_structure.c

scheduler.o: scheduler.h
	$(CC) $(CFLAGS) scheduler.c

test: test.c data_structure.o my_pthread.o scheduler.o
	$(CC) -pthread -g -o test data_structure.o my_pthread.o scheduler.o test.c

benchmark: benchmark.c data_structure.o my_pthread.o scheduler.o
	$(CC) -pthread -g -o benchmark data_structure.o my_pthread.o scheduler.o benchmark.c -lm

clean:
	rm -rf *.o *.a test benchmark
