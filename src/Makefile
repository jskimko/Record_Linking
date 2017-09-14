CC=gcc
CFLAGS=-O3 -Wall -fopenmp
OBJS=record_linker.o jarowinkler.o name_dict.o
DEPS=record_linker.h jarowinkler.h name_dict.h

all: record_linker

print: CFLAGS += -DPRINT
print: record_linker

record_linker: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

record_linker.o: record_linker.c record_linker.h
	$(CC) -c $< $(CFLAGS) -o $@

jarowinkler.o: jarowinkler.c jarowinkler.h
	$(CC) -c $< $(CFLAGS) -o $@

name_dict.o: name_dict.c name_dict.h
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o record_linker
