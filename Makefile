CFLAGS   = -Wall -Wextra -g
CPPFLAGS = -D_GNU_SOURCE
SRCS     = $(wildcard *.c *.h)
PROGS    = $(basename $(wildcard *.c))

all: $(PROGS)

%: %.c ns_util.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

format:
	clang-format -i $(SRCS)

clean:
	rm -f $(PROGS)

.PHONY: all clean format
