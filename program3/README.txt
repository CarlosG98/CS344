How to compile smallsh:


	In the zip folder should be a Makefile. enter the command: make
	this should create an executable called smallsh. 
	to run smallsh, enter the command: ./smallsh

	in the event that Makefile does not create an executable, 
	enter the command: gcc --std=gnu99 -o smallsh main.c dynarray.c
	then run smallsh by entering: ./smallsh