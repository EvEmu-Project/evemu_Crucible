
CXXFLAGS=$(CXXFLAGS_COMMON) -ggdb -I/usr/include/mysql \
  -I. -fno-inline
LDFLAGS=-lpcap -lz -rdynamic -O2 -ggdb -pipe -ldl -lpthread \
  -L../libs/libnids/src -lnids \
  -L../libs/libnet/src -lnet \
  -lutil -lreadline \
  -L/usr/lib/mysql -lmysqlclient
# `python-config`

everything: all

include .depend

include Makefile.common


