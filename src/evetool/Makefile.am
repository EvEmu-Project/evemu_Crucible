INCLUDES += -I$(srcdir)/../../include/common/
INCLUDES += -I$(srcdir)/../../include/evetool/

bin_PROGRAMS = evetool stuff_extract

evetool_SOURCES = \
	main.cpp \
	TriFile.cpp \
	TriFile.h

evetool_LDADD = \
	$(srcdir)/../common/libcommon.a \
	$(srcdir)/../packets/libpackets.a \
	-lz \
	-lpthread \
	-lreadline

evetool_LDFLAGS = \
	@MYSQL_LDFLAGS@

stuff_extract_SOURCES = \
	stuff_extract.cpp

