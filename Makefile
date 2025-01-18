SRCDIR=./src
LIBDIR=./lib
INCDIR=./inc
BINDIR=./bin

CC=gcc
CFLAGS=-I$(INCDIR) -O2 -Wall -Wextra -Wpedantic -fsanitize=address,undefined
DFLAGS=-I$(INCDIR) -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined
EXE=jplc

TEST=test.jpl

SRCFILES=$(shell find $(SRCDIR) -name "*.c")
LIBFILES=$(shell find $(LIBDIR) -name "*.c")
HFILES=$(shell find $(INCDIR) -name "*.h")

$(EXE): $(SRCFILES) $(LIBFILES) $(HFILES)
	$(CC) $(CFLAGS) -o $(EXE) $(SRCFILES) $(LIBFILES)

debug:
	$(CC) $(DFLAGS) -o $(EXE) $(SRCFILES) $(LIBFILES)

run: $(EXE)
	./$(EXE) -l $(TEST)

.PHONY: clean

clean:
