ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = configure

SUBDIRS = \
	dep/gangsta \
	dep/tinyxml \
	dep/utf8cpp \
	dep/utils \
	src/common \
	src/xmlpktgen \
	src/eve-common \
	src/eve-server

if EVECOLLECTOR
SUBDIRS += \
	src/eve-collector
endif

if EVETOOL
SUBDIRS += \
	src/eve-tool
endif
