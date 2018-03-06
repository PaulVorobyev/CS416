CC = gcc
CFLAGS = -c -O3 -I../common
AR = ar -rc
RANLIB = ranlib

Target: my_pthread.a

my_pthread.a: my_pthread.o scheduler.o
	$(AR) libmy_pthread.a my_pthread.o scheduler.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(MAKE) -C ../asst2
	$(CC) -pthread $(CFLAGS) my_pthread.c -L../asst2 -lmy_malloc -I../common

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
