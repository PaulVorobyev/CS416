CC = gcc
CFLAGS = -c -O3
AR = ar -rc
RANLIB = ranlib

Target: os_common.a

os_common.a: data_structure.o
	$(AR) libos_common.a data_structure.o
	$(RANLIB) libos_common.a

data_structure.o: data_structure.h
	$(CC) $(CFLAGS) data_structure.c

benchmark: benchmark.c my_pthread.a
	$(CC) -g -o benchmark benchmark.c -lm -L. -lmy_pthread

run_test:
	make clean && make test && ./test

run_benchmark:
	make clean && make benchmark && ./benchmark jobs.txt

clean:
	rm -rf *.o *.a test benchmark
