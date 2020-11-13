#
# Makefile for ibtcl
#
IBROOT?=/opt/firebird
TCLROOT?=/usr
TCLVER?=8.6

VERSION = ibtcl-020
IBFLAGS = -I$(IBROOT)/include
TCLFLAGS = -I$(TCLROOT)/include -I$(TCLROOT)/include/tcl
#DEBUGFLAGS = -ggdb
CFLAGS = -fPIC $(IBFLAGS) $(TCLFLAGS) $(DEBUGFLAGS)

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

testprep:
	pushd /tmp
	PATH=$(IBROOT)/bin:$(PATH)
	LD_LIBRARY_PATH=$(IBROOT)/lib:$(LD_LIBRARY_PATH)
	gsec -user SYSDBA -password masterkey -add ibtcltest -pw test
	gsec -user SYSDBA -password masterkey -add ibtcltestcyr -pw тест
	gsec -user SYSDBA -password masterkey -add ibtcltтест -pw тест
	isql -q -u SYSDBA -p masterkey -ch utf-8 -i test-create.sql
	popd

testrun:
	PATH=$(TCLROOT)/bin:$(PATH) \
	LD_LIBRARY_PATH=$(TCLROOT)/lib:$(IBROOT)/lib:$(LD_LIBRARY_PATH) \
	Ibtcldll=./libibtcl.so \
	FirebirdData=/tmp \
	tclsh test.tcl

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
