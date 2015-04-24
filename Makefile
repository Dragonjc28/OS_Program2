CC=gcc

liblwp.so:
	$(CC) -Wall -g -fpic -c lwp.c -o liblwp.o
	$(CC) -Wall -g -fpic -shared -o liblwp.so liblwp.o magic64.s

clean:
	rm liblwp.so
	rm liblwp.o
