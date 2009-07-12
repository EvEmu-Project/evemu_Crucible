INCLUDES += -I$(srcdir)/../3rdparty/
INCLUDES += -I$(srcdir)/../../include/common/
INCLUDES += -I$(srcdir)/../../include/xmlpktgen/
INCLUDES += -I$(srcdir)/../../dep/tinyxml/

bin_PROGRAMS = xmlpktgen

xmlpktgen_SOURCES = \
	../common/logsys.cpp \
	../common/logsys.h \
	../common/logtypes.h \
	../common/misc.cpp \
	../common/misc.h \
	../common/MiscFunctions.cpp \
	../common/MiscFunctions.h \
	CloneGenerator.cpp \
	CloneGenerator.h \
	ConstructGenerator.cpp \
	ConstructGenerator.h \
	DecodeGenerator.cpp \
	DecodeGenerator.h \
	DestructGenerator.cpp \
	DestructGenerator.h \
	DumpGenerator.cpp \
	DumpGenerator.h \
	EncodeGenerator.cpp \
	EncodeGenerator.h \
	Generator.cpp \
	Generator.h \
	HeaderGenerator.cpp \
	HeaderGenerator.h \
	ImplGenerator.cpp \
	ImplGenerator.h \
	main.cpp \
	XMLPacketGen.cpp \
	XMLPacketGen.h

#-lcommon

xmlpktgen_LDADD = \
	-lz \
	-ldl \
	-lpthread \
	-lpcap \
	-ltinyxml

xmlpktgen_LDFLAGS = \
	-L$(srcdir)/../common/ \
	-L$(srcdir)/../../dep/tinyxml \
	-rdynamic \
	@MYSQL_LDFLAGS@ \
	@AM_LDFLAGS@

