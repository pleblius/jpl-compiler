SRCDIR=./src
LIBDIR=./lib
INCDIR=./inc
BINDIR=./bin
DEBUGDIR=./debug

CC=gcc
RELEASEFLAGS=-I$(INCDIR) -O3 -Wall -Wextra
TESTFLAGS=-I$(INCDIR) -O2 -Wall -Wextra -fsanitize=address,undefined
DEBUGFLAGS=-I$(INCDIR) -g -Wall -Wextra -fsanitize=address,undefined
CFLAGS=$(RELEASEFLAGS)

EXE=jplc
DEBUG=jplc-debug
TEST=test.jpl
FLAGS=-p

_LIB = stringops token vector dict vecs astnode
_SRC = main lexer printer error parser typecheck

LIBDEPS = $(patsubst %,$(INCDIR)/%.h,$(_LIB))
SRCDEPS = $(patsubst %,$(INCDIR)/%.h,$(_SRC))

LIBOBJ = $(patsubst %,$(BINDIR)/%.o,$(_LIB))
SRCOBJ = $(patsubst %,$(BINDIR)/%.o,$(_SRC))

$(EXE): $(LIBOBJ) $(SRCOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(LIBOBJ): $(BINDIR)/%.o: $(LIBDIR)/%.c $(LIBDEPS)
	@mkdir -p $(BINDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SRCOBJ): $(BINDIR)/%.o: $(SRCDIR)/%.c $(LIBDEPS) $(SRCDEPS)
	@mkdir -p $(BINDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

DEBUGLIBOBJ = $(patsubst %,$(DEBUGDIR)/%.o,$(_LIB))
DEBUGSRCOBJ = $(patsubst %,$(DEBUGDIR)/%.o,$(_SRC))

$(DEBUG): $(DEBUGLIBOBJ) $(DEBUGSRCOBJ)
	$(CC) -o $@ $^ $(DEBUGFLAGS)

$(DEBUGLIBOBJ): $(DEBUGDIR)/%.o: $(LIBDIR)/%.c $(LIBDEPS)
	@mkdir -p $(DEBUGDIR)
	$(CC) -c -o $@ $< $(DEBUGFLAGS)

$(DEBUGSRCOBJ): $(DEBUGDIR)/%.o: $(SRCDIR)/%.c $(LIBDEPS) $(SRCDEPS)
	@mkdir -p $(DEBUGDIR)
	$(CC) -c -o $@ $< $(DEBUGFLAGS)

all: $(EXE)

debug: $(DEBUG)

clean:
	@rm -f $(BINDIR)/*.o
	@rm -f $(DEBUGDIR)/*.o
	@rm -f *.txt
	@rm -f callgrind.*
	@rm -f *.out
	@rm -f *.s
	@find . -type f -name "*.Identifier" -delete
	@find . -type f -name "cachegrind.*" -delete


run: $(EXE)
	@./$(EXE) $(FLAGS) $(TEST)

lines:
	wc -l src/*.c
	wc -l lib/*.c
	wc -l inc/*.h
