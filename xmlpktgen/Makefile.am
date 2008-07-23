INCLUDES += -I$(srcdir)/../common/

bin_PROGRAMS = xmlpktgen

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
	Generator.cpp \
	Generator.h \
	HeaderGenerator.cpp \
	HeaderGenerator.h \
	ImplGenerator.cpp \
	ImplGenerator.h \
	Language.cpp \
	Language.h \
	main.cpp \
	XMLPacketGen.cpp \
	XMLPacketGen.h

xmlpktgen_LDADD = \
	-lcommon \
	-lz \
	-ldl \
	-lpthread \
	-lpcap \
	-ltinyxml

xmlpktgen_LDFLAGS = \
	-L$(srcdir)/../common/ \
	-rdynamic \
	@MYSQL_LDFLAGS@ \
	@AM_LDFLAGS@

