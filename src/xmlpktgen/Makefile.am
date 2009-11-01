bin_PROGRAMS = \
	xmlpktgen

xmlpktgen_SOURCES = \
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
	XMLPacketGen.h \
	XMLPktGenPCH.h

xmlpktgen_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	-I$(srcdir)/../../dep/ \
	-I$(srcdir)/../../include/common/ \
	-I$(srcdir)/../../include/xmlpktgen/

xmlpktgen_LDADD = \
	$(srcdir)/../../dep/tinyxml/libtinyxml.a \
	$(srcdir)/../common/libcommon.a
