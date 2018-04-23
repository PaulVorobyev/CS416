CC = gcc
CFLAGS = -O3 
AR = ar -rc
RANLIB = ranlib

make_asst1:
	$(MAKE) -C asst1

make_asst2:
	$(MAKE) -C asst2

asst1_test:
	$(MAKE) -C asst1
	$(MAKE) -C asst2
	$(MAKE) -C common
	$(CC) $(CFLAGS) -g -o bin/test asst1/test.c -Lasst1 -lmy_pthread -Lasst2 -lmy_malloc -Lcommon -los_common

asst1_benchmark:
	$(MAKE) -C asst1
	$(CC) $(CFLAGS) -g -o bin/benchmark asst1/benchmark.c -Lasst1 -lmy_pthread -Lasst2 -lmy_malloc

asst2_test:
	$(MAKE) -C asst1
	$(MAKE) -C asst2
	$(MAKE) -C common
	$(CC) $(CFLAGS) -g -o bin/test test.c -Lasst1 -lmy_pthread -Lasst2 -lmy_malloc -Lcommon -los_common

clean:
	rm -rf asst1/*.o asst1/*.a
	rm -rf asst2/*.o asst2/*.a
	rm -rf common/*.o common/*.a
	rm -rf bin/*.dSYM bin/test bin/benchmark
