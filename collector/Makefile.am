INCLUDES += -I$(srcdir)/../3rdparty/

bin_PROGRAMS = collector 

collector_SOURCES = \
	DestinyHandler.cpp \
	destiny_types.h \
	EVECollectDisp.cpp \
	EVECollectDisp.h \
	evecollector.cpp \
	EVEHandlers.cpp \
	EVEnids.cpp \
	method_types.h

collector_LDADD = \
	-lpcap \
	-lcommon \
	-lpackets \
	-lnids \
	-ldl \
	-lz \
	@MYSQL_LDFLAGS@ \
	@AM_LDFLAGS@

collector_LDFLAGS = \
	-pipe \
	-rdynamic \
	-L$(srcdir)/../common/ \
	-L$(srcdir)/../packets/ \
	@AM_LDFLAGS@

