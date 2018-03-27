CC = gcc
CFLAGS = -g -c -O3 -I../common
AR = ar -rc
RANLIB = ranlib

Target: my_malloc.a

my_malloc.a: virtual_memory.o my_malloc.o my_shalloc.o structures.o
	$(AR) libmy_malloc.a $^
	$(RANLIB) libmy_malloc.a

virtual_memory.o: virtual_memory.c virtual_memory.h
	$(CC) $(CFLAGS) $<

my_malloc.o: my_malloc.c my_malloc.h
	$(CC) $(CFLAGS) $<

my_shalloc.o: my_shalloc.c my_shalloc.h
	$(CC) $(CFLAGS) $<

structures.o: structures.c structures.h
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o *.a
