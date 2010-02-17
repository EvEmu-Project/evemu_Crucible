
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
	$(srcdir)/../eve-common/libevecommon.a \
	$(srcdir)/../eve-common/packets/libpackets.a \
	$(srcdir)/../common/libcommon.a \
	@MYSQL_LDFLAGS@ \
	-lpcap \
	-lnids \
	-ldl \
	-lz
