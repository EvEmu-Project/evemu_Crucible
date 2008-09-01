INCLUDES += -I$(srcdir)/../3rdparty/
INCLUDES += -I$(srcdir)/../common/

bin_PROGRAMS = xmlpktgen

xmlpktgen_SOURCES = \
	../common/PyRep.cpp \
	../common/logsys.cpp \
	../common/misc.cpp \
	../common/EVEUnmarshal.cpp \
	../common/EVEMarshal.cpp \
	../common/packet_functions.cpp \
	../common/EVEZeroCompress.cpp \
	../common/PyVisitor.cpp \
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
	-L$(srcdir)/../3rdparty/tinyxml \
	-rdynamic \
	@MYSQL_LDFLAGS@ \
	@AM_LDFLAGS@

