
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

collector_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	$(EVEMU_CXXFLAGS) \
	$(MYSQL_CFLAGS) \
	$(LIBNET_CXXFLAGS)

collector_LDADD = \
	$(srcdir)/../eve-common/libevecommon.a \
	$(srcdir)/../eve-common/packets/libpackets.a \
	$(srcdir)/../common/libcommon.a \
	$(MYSQL_LDFLAGS) \
	-lnet \
	-lpcap \
	-lnids \
	-ldl \
	-lz
