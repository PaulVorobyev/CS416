CC = gcc
CFLAGS = -g -c -O3 -I../common
AR = ar -rc
RANLIB = ranlib

Target: my_malloc.a

my_malloc.a: my_malloc.o virt_mem.o
	$(AR) libmy_malloc.a virt_mem.o my_malloc.o
	$(RANLIB) libmy_malloc.a

my_malloc.o: my_malloc.h
	$(CC) $(CFLAGS) my_malloc.c

virt_mem.o: virt_mem.h
	$(CC) $(CFLAGS) virt_mem.c

clean:
	rm -rf *.o *.a
