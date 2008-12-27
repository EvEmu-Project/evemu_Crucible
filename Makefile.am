ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = configure

SUBDIRS = dep src/xmlpktgen src/packets src/common src/server

if COLLECTOR
SUBDIRS+= src/collector
endif

if EVETOOL
SUBDIRS+= src/evetool
endif

