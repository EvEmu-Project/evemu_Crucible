
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
	$(srcdir)/../common/libcommon.a \
	$(srcdir)/../packets/libpackets.a \
	-lpcap \
	-lnids \
	-ldl \
	-lz \
	@MYSQL_LDFLAGS@ \
	@AM_LDFLAGS@

collector_LDFLAGS = \
	-pipe \
	-rdynamic \
	@AM_LDFLAGS@

