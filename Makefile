
INSTALLPREFIX	?= /usr/local

CFLAGS		+= -Wall -Wextra
CFLAGS		+= -Werror
LDFLAGS		+= -L. -lnows

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
TESTS		+= test/clisrv/cli test/clisrv/srv test/clisrv/cli2 test/clisrv/srv2
TARGETS		= $(LIB) $(TESTS)

SRC		+= src/nows.c $(A)/wsb64enc.c $(A)/wsc.c $(A)/wssha1.c
SRCDIRS		= $(A)/.. $(A) src
INCLUDES	= $(SRCDIRS:%=-I%)
CFLAGS		+= $(INCLUDES)

default: help

all: lib doc ## build library and documentation

lib: $(TARGETS:%.c=%) ## build library only

%.o: %.c
	$(CC) $(CFLAGS) -MD -MF $(@:%.o=%.d) -c $< -o $@

%: %.o
	$(CC) $< -o $@ $(LDFLAGS)

libnows.a: $(SRC:%.c=%.o)
	$(AR) qs $@ $^

analyze: $(SRC:%.c=%.analyze) $(TESTS:%=%.analyze) ## run clang's --analyze
 
%.analyze: %.c
	$(CC) $(CFLAGS) --analyze -c $< -o $@

clean: ## remove library
	rm -f $(LIB) $(TARGETS)
	rm -f $(SRC:%.c=%.d) $(SRC:%.c=%.o) $(SRC:%.c=%.analyze)
	rm -f $(TESTS:%=%.d) $(TESTS:%=%.o) $(TESTS:%=%.analyze)

mrproper: clean ## remove library and doc
	rm -rf doc README.md

.SUFFIXES: # no implicit rules
.PRECIOUS: $(TARGETS:%=%.o)
-include $(SRCDIRS:%=%/*.d) $(TESTS:%=%.d)

help: ## this help
	@echo "rules are:"
	@sed -n 's,\(^[^:]*:\)[^#]*##\(.*\),\t\1 \2,p' < $(firstword $(MAKEFILE_LIST))
	@echo "make variables:"
	@echo "	DEBUG=1 - compile in debug mode"
	@echo "	INSTALLPREFIX=$(INSTALLPREFIX) - default"
	@echo "	CFLAGS LDFLAGS CC AR"

.PHONY: doc
doc: doc/nowsread.3 doc/nows.htmlhelp/index.html doc/nows.pdf doc/nows.text README.md ## generate documentation

doc/nows.asciidoc: src/nows.asciidoc
	mkdir -p doc
	cp $< $@

A2X = a2x -v --doctype manpage
A2XCHECK = 2>&1 > doc/doc.log && rm -f doc/doc.log || { rm -f $@; cat doc/doc.log; false; }

doc/nowsread.3: doc/nows.asciidoc
	$(A2X) --format manpage $< $(A2XCHECK)

doc/nows.htmlhelp/index.html: doc/nows.asciidoc
	$(A2X) --format htmlhelp $< $(A2XCHECK)

doc/nows.pdf: doc/nows.asciidoc
	$(A2X) --format pdf $< $(A2XCHECK)

doc/nows.text: doc/nows.asciidoc
	$(A2X) --format text $< $(A2XCHECK)

README.md: doc/nows.text
	cp $< $@

# do not check install dependencies because make install may be run with 'sudo'

install: install-lib install-doc ## install lib and doc (sudo make INSTALLPREFIX=/some/path install)

install-lib: ## lib must be built
	mkdir -p $(INSTALLPREFIX)/lib $(INSTALLPREFIX)/include
	cp src/nows.h $(INSTALLPREFIX)/include
	cp libnows.a $(INSTALLPREFIX)/lib

install-doc: ## doc must be built
	mkdir -p $(INSTALLPREFIX)/share/man/man3
	cp doc/nows*.3 $(INSTALLPREFIX)/share/man/man3
