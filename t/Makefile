include ../Makefile.defs	# for CC, OPT, etc.

LIBDIR  = ..
INCDIR  = ..
LIB     = btparse
LIBFILE = lib$(LIB).a
CPPFLAGS = -I$(INCDIR)

LDFLAGS = -L$(LIBDIR) -l$(LIB) $(EXTRA_LDFLAGS)

TESTLIB_SRC = testlib.c
TESTLIB_OBJ = testlib.o
TESTLIB_HDR = testlib.h

SIMPLE = simple_test
READ   = read_test

OBJ    = $(SIMPLE).o $(READ).o $(TESTLIB_OBJ)
EXEC   = $(SIMPLE) $(READ)

all test : simple read

read : $(READ)
	./$(READ) 2> /dev/null

simple : $(SIMPLE)
	./$(SIMPLE) 2> /dev/null

$(SIMPLE) : $(SIMPLE).o $(TESTLIB_OBJ) lib
	$(CC) $(CFLAGS) -o $@ $(SIMPLE).o $(TESTLIB_OBJ) $(LDFLAGS)

$(READ) : $(READ).o $(TESTLIB_OBJ) lib
	$(CC) $(CFLAGS) -o $@ $(READ).o $(TESTLIB_OBJ) $(LDFLAGS)

lib :
	cd $(LIBDIR) ; $(MAKE) $(LIBFILE)

clean :
	rm -f $(OBJ) $(EXEC) core logfile
