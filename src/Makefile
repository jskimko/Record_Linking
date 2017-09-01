CC=gcc
CFLAGS=-O3
OBJS=extract.o jarowinkler.o
DEPS=extract.h jarowinkler.h

all: extract

print: CFLAGS += -DPRINT
print: extract

extract: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o extract
