ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = configure

SUBDIRS = dep xmlpktgen packets common server

if COLLECTOR
SUBDIRS+= collector
endif

if EVETOOL
SUBDIRS+= evetool
endif

