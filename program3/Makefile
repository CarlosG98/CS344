CC=gcc --std=gnu99 -g

all: smallsh

smallsh:	main.c dynarray.o
	$(CC) main.c dynarray.o -o smallsh

dynarray.o:	dynarray.c dynarray.h
	$(CC) -c dynarray.c

clean:
	rm -f smallsh dynarray.o
