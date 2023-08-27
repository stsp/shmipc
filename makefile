CFLAGS ?= -Wall -Wmissing-prototypes
OS ?= $(shell uname -s)
SOURCES = $(OS)/shm.c rwlock.c condvar.c ipc.c pgalloc.c main.c
_PROG = shmipc
ifeq ($(OS),Linux)
SOURCES += $(OS)/shlock.c
CFLAGS += -ggdb3
PROG = $(_PROG)
else
CC = i586-pc-msdosdjgpp-gcc
CFLAGS += -march=i686
PROG = $(_PROG).exe
endif
OBJECTS = $(SOURCES:.c=.o)

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $^

clean:
	rm -f `find . -name '*.o'` $(PROG)
