include ../Makefile.defs	# for CC, OPT, etc.

LIBDIR  = ..
INCDIR  = ..
LIB     = btparse
LIBFILE = lib$(LIB).a
CFLAGS  = $(OPT) -I$(INCDIR)
LDFLAGS = -L$(LIBDIR) $(OTHERLIBDIRS) -l$(LIB) $(OTHERLIBS)

test : simple_test
	./simple_test 2> /dev/null

simple_test : simple_test.o lib
	$(CC) $(CFLAGS) -o $@ simple_test.o $(LDFLAGS)

lib :
	cd $(LIBDIR) ; make $(LIBFILE)

clean :
	rm -f simple_test.o core logfile

scrub :
	rm -f simple_test
