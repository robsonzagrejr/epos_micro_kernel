# EPOS ARMv7 Architecture Makefile

include ../../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc | grep -v _init | grep -v _test))
CRTS := $(subst .S,.o,$(shell find *.S | grep crt))
CRTS += $(subst .c,.o,$(shell find *.c | grep crt))
CRTSI := $(subst .S,.s,$(shell find *.S | grep crt))
INITS := $(subst .cc,.o,$(shell find *.cc | grep _init))

all:		crts $(LIBARCH) $(LIBINIT)

crts:		$(CRTS)
		$(INSTALL) $(ARCH)_crt0.o $(LIB)/crt0_$(MMOD).o
		$(INSTALL) $(ARCH)_crtbegin.o $(LIB)/crtbegin_$(MMOD).o
		$(INSTALL) $(ARCH)_crtend.o $(LIB)/crtend_$(MMOD).o

.INTERMEDIATE:	$(CRTSI)

$(LIBARCH):	$(LIBARCH)($(OBJS))

$(LIBINIT):	$(LIBINIT)($(INITS))

clean:
		$(CLEAN) *.o *.s *_test
