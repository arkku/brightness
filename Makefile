CC=clang
ARCHFLAGS=-arch i386 -m32
CFLAGS=-Wall -std=c99 -pedantic -Wextra $(ARCHFLAGS) \
       -DMACOSX_DEPLOYMENT_TARGET=10.5 -Os 
LDFLAGS=-Os $(ARCHFLAGS)
FRAMEWORKS=-framework IOKit -framework ApplicationServices

OBJS = brightness.o
BINPATH = ./
BINS = $(BINPATH)brightness
TESTER = valgrind

.PHONY: all clean distclean test

all: $(BINS) $(LIB)

$(BINS): | $(BINPATH)
$(LIB): | $(LIBPATH)

$(BINPATH)brightness: brightness.o
	$(CC) $(LDFLAGS) -o $@ $+ $(FRAMEWORKS)

$(sort $(BINPATH) $(LIBPATH)):
	@mkdir -p $@

test: brightness
	@$(TESTER) $(BINPATH)brightness

clean:
	rm -f $(OBJS)

distclean: | clean
	rm -f $(BINS)
	@rmdir $(BINPATH) >/dev/null 2>/dev/null || true

