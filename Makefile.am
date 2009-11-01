ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = configure

SUBDIRS = \
	dep \
	src/common \
	src/xmlpktgen \
	src/eve-common \
	src/eve-server

if COLLECTOR
SUBDIRS += \
	src/eve-collector
endif

if EVETOOL
SUBDIRS += \
	src/eve-tool
endif
