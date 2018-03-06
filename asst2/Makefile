CC = gcc
CFLAGS = -c -O3 -I../common
AR = ar -rc
RANLIB = ranlib

Target: my_malloc.a

my_malloc.a: my_malloc.o
	$(AR) libmy_malloc.a my_malloc.o
	$(RANLIB) libmy_malloc.a

my_malloc.o: my_malloc.h
	$(CC) $(CFLAGS) my_malloc.c

clean:
	rm -rf *.o *.a
