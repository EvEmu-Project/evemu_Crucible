noinst_LIBRARIES = \
	libcommon.a

libcommon_a_SOURCES = \
	database/dbcore.cpp \
	database/dbcore.h \
	database/dbtype.cpp \
	database/dbtype.h \
	log/LogNew.cpp \
	log/LogNew.h \
	log/logsys.cpp \
	log/logsys.h \
	log/logtypes.h \
	network/NetUtils.cpp \
	network/NetUtils.h \
	network/Socket.cpp \
	network/Socket.h \
	network/StreamPacketizer.cpp \
	network/StreamPacketizer.h \
	network/TCPConnection.cpp \
	network/TCPConnection.h \
	network/TCPServer.cpp \
	network/TCPServer.h \
	threading/Mutex.cpp \
	threading/Mutex.h \
	utils/Buffer.h \
	utils/crc32.cpp \
	utils/crc32.h \
	utils/Deflate.cpp \
	utils/Deflate.h \
	utils/DirWalker.cpp \
	utils/DirWalker.h \
	utils/gpoint.h \
	utils/misc.cpp \
	utils/misc.h \
	utils/RefPtr.h \
	utils/seperator.h \
	utils/Singleton.h \
	utils/timer.cpp \
	utils/timer.h \
	utils/utils_hex.cpp \
	utils/utils_hex.h \
	utils/utils_string.cpp \
	utils/utils_string.h \
	utils/utils_time.cpp \
	utils/utils_time.h \
	utils/XMLParser.cpp \
	utils/XMLParser.h \
	common.cpp \
	common.h \
	CommonPCH.h

libcommon_a_CXXFLAGS = \
	$(MYSQL_CFLAGS) \
	-I$(srcdir)/../../dep/ \
	-I$(srcdir)/../../include/common/
