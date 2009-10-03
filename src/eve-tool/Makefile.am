bin_PROGRAMS = \
	evetool \
	stuff_extract

evetool_SOURCES = \
	main.cpp \
	TriFile.cpp \
	TriFile.h

evetool_CXXFLAGS = \
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
	$(srcdir)/../../dep/utils/libutils.a

evetool_LDFLAGS = \
	@MYSQL_LDFLAGS@ \
	-lz \
	-lpthread \
	-lreadline

stuff_extract_SOURCES = \
	stuff_extract.cpp

stuff_extract_CXXFLAGS = \
	-I$(srcdir)/../../include/common/
