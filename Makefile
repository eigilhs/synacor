VPATH = src
CFLAGS = -O3

.PHONY: distclean

all: vm

distclean:
	$(RM) vm
