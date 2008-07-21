
CXXFLAGS=$(CXXFLAGS_COMMON) -I/usr/include/mysql \
 -DUSE_RUNTIME_EXCEPTIONS -fnon-call-exceptions
LDFLAGS=-lz -rdynamic -O2 -ggdb -pipe -ldl -lpthread \
  -L/usr/lib/mysql -lmysqlclient

everything:
	$(MAKE) -C ../xmlpktgen
	$(MAKE) all

include .depend

include Makefile.common

