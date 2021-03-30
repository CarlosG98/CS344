how to compile:

	in the zip folder should be a Makefile. enter the following command: make
	this should compile and create an executable called "line_processor"
	enter the following command to run the line processor: ./line_processor

	in the event that the Makefile should not work, enter the command: gcc --std=gnu99 -g -pthread -o line_processor main.c
	then run line_processor by entering: ./line_processor