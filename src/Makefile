CC=gcc
CFLAGS=-O3 -Wall
OBJS=record_linker.o jarowinkler.o name_dict.o
DEPS=record_linker.h jarowinkler.h name_dict.h

all: record_linker

print: CFLAGS += -DPRINT
print: record_linker

record_linker: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o record_linker
