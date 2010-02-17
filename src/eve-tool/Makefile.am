bin_PROGRAMS = \
	evetool

evetool_SOURCES = \
	Commands.cpp \
	EVEToolPCH.h \
	main.cpp \
	main.h

evetool_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	$(MYSQL_CFLAGS) \
	-I$(srcdir)/../../dep/ \
	-I$(srcdir)/../../include/common/ \
	-I$(srcdir)/../../include/eve-common/ \
	-I$(srcdir)/../../include/eve-tool/ \
	-I$(srcdir)/../eve-common/

evetool_LDADD = \
	$(srcdir)/../eve-common/libevecommon.a \
	$(srcdir)/../eve-common/packets/libpackets.a \
	$(srcdir)/../common/libcommon.a \
	$(srcdir)/../../dep/utils/libutils.a \
	@MYSQL_LDFLAGS@ \
	-lz \
	-lpthread
