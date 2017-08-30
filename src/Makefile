CC=gcc
CFLAGS=-O3
OBJS=extract.o
DEPS=extract.h

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) -o $@

extract: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f *.o extract
