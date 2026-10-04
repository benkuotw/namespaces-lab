CFLAGS = -Wall -Wextra -g
SRCS   = $(wildcard *.c *.h)
PROGS  = $(basename $(wildcard *.c))

all: $(PROGS)

$(PROGS): ns_util.h

format:
	clang-format -i $(SRCS)

clean:
	rm -f $(PROGS)

.PHONY: all clean format
