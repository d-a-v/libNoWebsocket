
INSTALLDIR	= /usr/local

CFLAGS		+= -Wall -Wextra
CFLAGS		+= -Werror
LDFLAGS		= -L. -lnows

ifeq ($(DEBUG),)
CFLAGS		+= -O2
else
CFLAGS		+= -O0 -ggdb -DDEBUG=1
#CFLAGS		+= -fstack-protection
endif

A		= Arduino/utility

LIB		= libnows.a
TESTS		+= test/sha1+b64/b64enctest
TESTS		+= test/sha1+b64/sha1test
TESTS		+= test/nowsecho/nowsecho
TESTS		+= test/nowseval/nowseval
TESTS		+= test/3js/interact
TARGETS		= $(LIB) $(TESTS)

SRC		+= src/nows.c $(A)/wsb64enc.c $(A)/wsc.c $(A)/wssha1.c
SRCDIRS		= $(A)/.. $(A) src
INCLUDES	= $(SRCDIRS:%=-I%)
CFLAGS		+= $(INCLUDES)

all: buildall help

buildall: $(TARGETS:%.c=%)

%.o: %.c
	$(CC) $(CFLAGS) -MD -MF $(@:%.o=%.d) -c $< -o $@

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

libnows.a: $(SRC:%.c=%.o)
	$(AR) qs $@ $^

analyze: $(SRC:%.c=%.analyze) $(TESTS:%=%.analyze) ## run clang's --analyze
 
%.analyze: %.c
	$(CC) $(CFLAGS) --analyze -c $< -o $@

clean: ## remove generated files except doc
	rm -f $(LIB) $(TARGETS)
	rm -f $(SRC:%.c=%.d) $(SRC:%.c=%.o) $(SRC:%.c=%.analyze)
	rm -f $(TESTS:%=%.d) $(TESTS:%=%.o) $(TESTS:%=%.analyze)

.SUFFIXES: # no implicit rules
.PRECIOUS: $(TARGETS:%=%.o)
-include $(SRCDIRS:%=%/*.d) $(TESTS:%=%.d)

help: ## this help
	@echo "-----------"
	@echo "rules are:"
	@sed -n 's,\(^[^:]*:\)[^#]*##\(.*\),\1 \2,p' < $(firstword $(MAKEFILE_LIST))
	@echo "make options:"
	@echo "	DEBUG=1 - compile in debug mode"

doc: nows.3 nows.html README.md ## generate documentation

nows.3: src/nows.tex latex2man
	latex2man/latex2man -M $< $@

nows.html: src/nows.tex latex2man
	latex2man/latex2man -H $< $@

README.md: nows.html
	cp nows.html README.md

latex2man:
	( curl https://www.informatik-vollmer.de/software/latex2man-1.27.tar.gz || \
	  wget -O - https://www.informatik-vollmer.de/software/latex2man-1.27.tar.gz ) | tar xfz -

install: install-lib install-doc

install-lib: libnows.a
	mkdir -p $(INSTALLDIR)/lib $(INSTALLDIR)/include
	cp src/nows.h $(INSTALLDIR)/include
	cp libnows.a $(INSTALLDIR)/lib

install-doc: doc
	mkdir -p $(INSTALLDIR)/share/man/man3
	cp nows.3 $(INSTALLDIR)/share/man/man3
	for i in nowsread nowswrite nowsclose nows_simulate_client; do ln -sf nows.3 $(INSTALLDIR)/share/man/man3/$$i.3; done
