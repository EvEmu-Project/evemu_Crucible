
bin_PROGRAMS = evetool stuff_extract

evetool_SOURCES = \
	main.cpp \
	TriFile.cpp \
	TriFile.h

evetool_LDADD = \
	-lcommon \
	-lpackets \
	-lutil \
	-lz \
	-lpthread \
	-lreadline

evetool_LDFLAGS = \
	-L$(srcdir)/../common/ \
	-L$(srcdir)/../packets/

stuff_extract_SOURCES = \
	stuff_extract.cpp

