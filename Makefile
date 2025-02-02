SRCDIR=./src
PRODDIR=./src/prods
LIBDIR=./lib
INCDIR=./inc
BINDIR=./bin

CC=gcc
RELEASEFLAGS=-I$(INCDIR) -O3 -Wall -Wextra -Wpedantic
DEBUGFLAGS=-I$(INCDIR) -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined
CFLAGS=$(RELEASEFLAGS)

EXE=jplc
TEST=test.jpl
FLAGS=-p

_LIB = stringops token vector error
_PARSE = parser prod
_PRODS = arg cmd expr lvalue stmt type bind
_SRC = compiler lexer parser

LIBDEPS = $(patsubst %,$(INCDIR)/%.h,$(_LIB))
PARSEDEPS = $(patsubst %,$(INCDIR)/%.h,$(_PARSE))
PRODDEPS = $(patsubst %,$(INCDIR)/%.h,$(_PRODS))
SRCDEPS = $(patsubst %,$(INCDIR)/%.h,$(_SRC))

LIBOBJ = $(patsubst %,$(BINDIR)/%.o,$(_LIB))
PRODSOBJ = $(patsubst %,$(BINDIR)/%.o,$(_PRODS))
SRCOBJ = $(patsubst %,$(BINDIR)/%.o,$(_SRC))

$(EXE): $(LIBOBJ) $(SRCOBJ) $(LIBOBJ) $(PRODSOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(LIBOBJ): $(BINDIR)/%.o: $(LIBDIR)/%.c $(LIBDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PRODSOBJ): $(BINDIR)/%.o: $(PRODDIR)/%.c $(LIBDEPS) $(PARSEDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SRCOBJ): $(BINDIR)/%.o: $(SRCDIR)/%.c $(LIBDEPS) $(SRCDEPS) $(LEXDEPS) $(PARSEDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	@rm -f $(BINDIR)/*.o
	@rm -f *.txt
	@rm -f callgrind.*
	@rm -rf *.out
	@rm -rf *.s

run: $(EXE)
	@./$(EXE) $(FLAGS) $(TEST)

debug: CFLAGS=$(DEBUGFLAGS)
debug: $(EXE)

lines:
	wc -l src/*.c
	wc -l src/prods/*.c
	wc -l lib/*.c
	wc -l inc/*.h
