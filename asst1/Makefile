CC = gcc
CFLAGS = -c
AR = ar -rc
RANLIB = ranlib

Target: my_pthread.a

my_pthread.a: my_pthread.o data_structure.o scheduler.o
	$(AR) libmy_pthread.a my_pthread.o data_structure.o scheduler.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread $(CFLAGS) my_pthread.c

data_structure.o: data_structure.h
	$(CC) $(CFLAGS) data_structure.c

scheduler.o: scheduler.h
	$(CC) $(CFLAGS) scheduler.c

test: test.c data_structure.o my_pthread.a
	$(CC) -g -o test data_structure.o test.c -L. -lmy_pthread

benchmark: benchmark.c my_pthread.a
	$(CC) -g -o benchmark benchmark.c -lm -L. -lmy_pthread

run_test:
	make clean && make test && ./test

run_benchmark:
	make clean && make benchmark && ./benchmark jobs.txt

clean:
	rm -rf *.o *.a test benchmark
