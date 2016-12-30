VPATH = src
CFLAGS = -O3

.PHONY: all distclean

all: vm

distclean:
	$(RM) vm
