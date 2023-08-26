CFLAGS ?= -Wall -Wmissing-prototypes -ggdb3
OS ?= $(shell uname -s)
SOURCES = $(OS)/shm.c $(OS)/shlock.c rwlock.c condvar.c ipc.c pgalloc.c main.c
OBJECTS = $(SOURCES:.c=.o)
PROG = shmipc

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $^

clean:
	rm -f `find . -name '*.o'` $(PROG)
