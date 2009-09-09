INCLUDES += -I$(srcdir)/../../dep/tinyxml/
INCLUDES += -I$(srcdir)/../../include/common/
INCLUDES += -I$(srcdir)/../../include/xmlpktgen/

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
	Generator.h \
	HeaderGenerator.cpp \
	HeaderGenerator.h \
	main.cpp \
	XMLPacketGen.cpp \
	XMLPacketGen.h

xmlpktgen_LDADD = \
	$(srcdir)/../../dep/tinyxml/libtinyxml.a

xmlpktgen_LDFLAGS = \
	-rdynamic \
	@AM_LDFLAGS@

