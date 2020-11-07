CC=gcc --std=gnu99 -g

all: movies

movies: main.c dynarray.o
	$(CC) main.c dynarray.o -o movies

dynarray.o: dynarray.c dynarray.h
	$(CC) -c dynarray.c

clean:
	rm -f movies dynarray.o
