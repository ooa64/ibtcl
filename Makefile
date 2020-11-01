#
# Makefile for ibtcl
#
IBROOT=.
TCLROOT=/opt/tcl
TCLVER=8.6

VERSION = ibtcl-011
IBFLAGS = -I$(IBROOT)/include
TCLFLAGS = -I$(TCLROOT)/include
#DEBUGFLAGS = -ggdb
CFLAGS = $(IBFLAGS) $(TCLFLAGS) $(DEBUGFLAGS)

TCLLIB = -L$(TCLROOT)/lib -ltcl$(TCLVER) -lm -ldl
IBLIB = -L$(IBROOT)/lib -lfbclient
# staticly linked IB lib
#IBLIB = -L$(IBROOT)/lib -lgds
# dynamicly linked IB lib
#IBLIB = -lgdslib

DISTR = COPYING Makefile Makefile.bor TODO cmd.c ibtcl.c ibtcl.h\
		ibtcl.html ibtcl.txt ibtclInt.h ibtclsh.c id.c

TARGETS = ibtclsh libibtcl.a libibtcl.so

OBJs=ibtcl.o cmd.o id.o

all: $(TARGETS)

ibtclsh: $(OBJs) ibtclsh.o
	gcc $(CFLAGS) -o $@ $^ $(TCLLIB) $(IBLIB)

libibtcl.a: $(OBJs)
	rm -f $@
	ar cr $@ $^

libibtcl.so: $(OBJs)
        gcc -shared -o $@ $^ $(TCLLIB) $(IBLIB)

$(OBJs): ibtclInt.h Makefile

clean:
	rm -f $(TARGETS) *.o core $(VERSION).tgz

tar:
	make clean
	tar -zcf $(VERSION).tgz $(DISTR)
