#
# Makefile for ibtcl
#
VERSION = ibtcl-01
IBFLAGS = -I/usr/ibase/ib4.0-linux/include
TCLFLAGC = 
#DEBUGFLAGS = -ggdb
CFLAGS = $(IBFLAGS) $(TCLFLAGS) $(DEBUGFLAGS)

TCLLIB = -ltcl -lm -ldl
# staticly linked IB lib
#IBLIB = -L/usr/ibase/ib4.0-linux/lib -lgds
# dynamicly linked IB lib
IBLIB = -lgdslib


DISTR = COPYING Makefile Makefile.bor TODO cmd.c ibtcl.c ibtcl.h\
		ibtcl.html ibtcl.txt ibtclInt.h ibtclsh.c id.c

TARGETS = ibtclsh libibtcl.a

OBJs=ibtcl.o cmd.o id.o

all: $(TARGETS)

ibtclsh: $(OBJs) ibtclsh.o
	gcc $(CFLAGS) -o $@ $^ $(TCLLIB) $(IBLIB)

libibtcl.a: $(OBJs)
	rm -f $@
	ar cr $@ $^

$(OBJs): ibtclInt.h Makefile

clean:
	rm -f $(TARGETS) *.o core $(VERSION).tgz

tar:
	make clean
	tar -zcf $(VERSION).tgz $(DISTR)
