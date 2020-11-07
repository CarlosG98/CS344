How to compile:

	zip file should contain a makefile labeled as "Makefile". Enter the following command into the terminal: make
	the command should compile the code and create an executable. To run the code, enter: ./movies movie_sample_1.csv
	
	In the event the makefile will not compile correctly, enter the command: gcc --std=gnu99 -o movies main.c dynarray.c