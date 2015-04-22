CC=gcc
ROOT_DIR= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SOURCES= malloc.c
LIBFLAGS= -g -Wall
liblwp.so:
	$(CC) -Wall -g -fpic -c lwp.c -o liblwp.o
	$(CC) -Wall -g -fpic -shared -o liblwp.so liblwp.o magic64.s

clean:
	rm liblwp.so
	rm liblwp.o
