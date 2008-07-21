
CXXFLAGS=-ggdb -O -pipe -Wall -I../common -I/usr/include/mysql \
  -I. -fno-inline \
  -I../libs/libnids/src
LDFLAGS=-lpcap -lz -rdynamic -O2 -ggdb -pipe -ldl -lpthread \
  -L../libs/libnids/src -lnids \
  -L../libs/libnet/src -lnet \
  -lutil \
  -L/usr/lib/mysql -lmysqlclient

everything: all

include .depend

include Makefile.common


