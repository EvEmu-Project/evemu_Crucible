#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "common" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/common.h"
     "${TARGET_INCLUDE_DIR}/CommonPCH.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/common.cpp" )

SET( database_INCLUDE
     "${TARGET_INCLUDE_DIR}/database/dbcore.h"
     "${TARGET_INCLUDE_DIR}/database/dbtype.h" )
SET( database_SOURCE
     "${TARGET_SOURCE_DIR}/database/dbcore.cpp"
     "${TARGET_SOURCE_DIR}/database/dbtype.cpp" )

SET( log_INCLUDE
     "${TARGET_INCLUDE_DIR}/log/LogNew.h"
     "${TARGET_INCLUDE_DIR}/log/logsys.h"
     "${TARGET_INCLUDE_DIR}/log/logtypes.h" )
SET( log_SOURCE
     "${TARGET_SOURCE_DIR}/log/LogNew.cpp"
     "${TARGET_SOURCE_DIR}/log/logsys.cpp" )

SET( network_INCLUDE
     "${TARGET_INCLUDE_DIR}/network/NetUtils.h"
     "${TARGET_INCLUDE_DIR}/network/Socket.h"
     "${TARGET_INCLUDE_DIR}/network/StreamPacketizer.h"
     "${TARGET_INCLUDE_DIR}/network/TCPConnection.h"
     "${TARGET_INCLUDE_DIR}/network/TCPServer.h" )
SET( network_SOURCE
     "${TARGET_SOURCE_DIR}/network/NetUtils.cpp"
     "${TARGET_SOURCE_DIR}/network/Socket.cpp"
     "${TARGET_SOURCE_DIR}/network/StreamPacketizer.cpp"
     "${TARGET_SOURCE_DIR}/network/TCPConnection.cpp"
     "${TARGET_SOURCE_DIR}/network/TCPServer.cpp" )

SET( threading_INCLUDE
     "${TARGET_INCLUDE_DIR}/threading/Mutex.h" )
SET( threading_SOURCE
     "${TARGET_SOURCE_DIR}/threading/Mutex.cpp" )

SET( utils_INCLUDE
     "${TARGET_INCLUDE_DIR}/utils/Buffer.h"
     "${TARGET_INCLUDE_DIR}/utils/crc32.h"
     "${TARGET_INCLUDE_DIR}/utils/Deflate.h"
     "${TARGET_INCLUDE_DIR}/utils/DirWalker.h"
     "${TARGET_INCLUDE_DIR}/utils/gpoint.h"
     "${TARGET_INCLUDE_DIR}/utils/Lock.h"
     "${TARGET_INCLUDE_DIR}/utils/misc.h"
     "${TARGET_INCLUDE_DIR}/utils/RefPtr.h"
     "${TARGET_INCLUDE_DIR}/utils/Seperator.h"
     "${TARGET_INCLUDE_DIR}/utils/Singleton.h"
     "${TARGET_INCLUDE_DIR}/utils/str2conv.h"
     "${TARGET_INCLUDE_DIR}/utils/timer.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_hex.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_string.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_time.h"
     "${TARGET_INCLUDE_DIR}/utils/XMLParser.h"
     "${TARGET_INCLUDE_DIR}/utils/XMLParserEx.h" )
SET( utils_SOURCE
     "${TARGET_SOURCE_DIR}/utils/crc32.cpp"
     "${TARGET_SOURCE_DIR}/utils/Deflate.cpp"
     "${TARGET_SOURCE_DIR}/utils/DirWalker.cpp"
     "${TARGET_SOURCE_DIR}/utils/misc.cpp"
     "${TARGET_SOURCE_DIR}/utils/Seperator.cpp"
     "${TARGET_SOURCE_DIR}/utils/str2conv.cpp"
     "${TARGET_SOURCE_DIR}/utils/timer.cpp"
     "${TARGET_SOURCE_DIR}/utils/utils_hex.cpp"
     "${TARGET_SOURCE_DIR}/utils/utils_string.cpp"
     "${TARGET_SOURCE_DIR}/utils/utils_time.cpp"
     "${TARGET_SOURCE_DIR}/utils/XMLParser.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${TINYXML_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${ZLIB_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

SOURCE_GROUP( "include"            FILES ${INCLUDE} )
SOURCE_GROUP( "include\\database"  FILES ${database_INCLUDE} )
SOURCE_GROUP( "include\\log"       FILES ${log_INCLUDE} )
SOURCE_GROUP( "include\\network"   FILES ${network_INCLUDE} )
SOURCE_GROUP( "include\\threading" FILES ${threading_INCLUDE} )
SOURCE_GROUP( "include\\utils"     FILES ${utils_INCLUDE} )

SOURCE_GROUP( "src"            FILES ${SOURCE} )
SOURCE_GROUP( "src\\database"  FILES ${database_SOURCE} )
SOURCE_GROUP( "src\\log"       FILES ${log_SOURCE} )
SOURCE_GROUP( "src\\network"   FILES ${network_SOURCE} )
SOURCE_GROUP( "src\\threading" FILES ${threading_SOURCE} )
SOURCE_GROUP( "src\\utils"     FILES ${utils_SOURCE} )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE}           ${SOURCE}
             ${database_INCLUDE}  ${database_SOURCE}
             ${log_INCLUDE}       ${log_SOURCE}
             ${network_INCLUDE}   ${network_SOURCE}
             ${threading_INCLUDE} ${threading_SOURCE}
             ${utils_INCLUDE}     ${utils_SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "CommonPCH.h"
                  "common.cpp" )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
