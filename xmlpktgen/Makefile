HCC=$(shell ccache -V 2>/dev/null)
ifneq (,$(findstring version,$(HCC)))
	CC=ccache g++
else
	CC=g++
endif

WFLAGS=-ansi -Wall -Wuninitialized -Wshadow -Wcast-qual -Wwrite-strings
CXXFLAGS=-ggdb -O -pipe $(WFLAGS) -O1
LDFLAGS=-lpcap -lz -rdynamic -O2 -ggdb -pipe -ldl -lpthread

everything: all

include .depend

include Makefile.common


