#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "common" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${COMPONENT_NAME}" )

#########
# Files #
#########
# common
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/common.h"
     "${COMPONENT_INCLUDE_DIR}/CommonPCH.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/common.cpp" )

# common/database
SET( database_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/database/dbcore.h"
     "${COMPONENT_INCLUDE_DIR}/database/dbtype.h" )
SET( database_SOURCE
     "${COMPONENT_SOURCE_DIR}/database/dbcore.cpp"
     "${COMPONENT_SOURCE_DIR}/database/dbtype.cpp" )

# common/log
SET( log_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/log/LogNew.h"
     "${COMPONENT_INCLUDE_DIR}/log/logsys.h"
     "${COMPONENT_INCLUDE_DIR}/log/logtypes.h" )
SET( log_SOURCE
     "${COMPONENT_SOURCE_DIR}/log/LogNew.cpp"
     "${COMPONENT_SOURCE_DIR}/log/logsys.cpp" )

# common/network
SET( network_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/network/NetUtils.h"
     "${COMPONENT_INCLUDE_DIR}/network/Socket.h"
     "${COMPONENT_INCLUDE_DIR}/network/StreamPacketizer.h"
     "${COMPONENT_INCLUDE_DIR}/network/TCPConnection.h"
     "${COMPONENT_INCLUDE_DIR}/network/TCPServer.h" )
SET( network_SOURCE
     "${COMPONENT_SOURCE_DIR}/network/NetUtils.cpp"
     "${COMPONENT_SOURCE_DIR}/network/Socket.cpp"
     "${COMPONENT_SOURCE_DIR}/network/StreamPacketizer.cpp"
     "${COMPONENT_SOURCE_DIR}/network/TCPConnection.cpp"
     "${COMPONENT_SOURCE_DIR}/network/TCPServer.cpp" )

# common/threading
SET( threading_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/threading/Mutex.h" )
SET( threading_SOURCE
     "${COMPONENT_SOURCE_DIR}/threading/Mutex.cpp" )

# common/utils
SET( utils_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/utils/Buffer.h"
     "${COMPONENT_INCLUDE_DIR}/utils/crc32.h"
     "${COMPONENT_INCLUDE_DIR}/utils/Deflate.h"
     "${COMPONENT_INCLUDE_DIR}/utils/DirWalker.h"
     "${COMPONENT_INCLUDE_DIR}/utils/gpoint.h"
     "${COMPONENT_INCLUDE_DIR}/utils/misc.h"
     "${COMPONENT_INCLUDE_DIR}/utils/RefPtr.h"
     "${COMPONENT_INCLUDE_DIR}/utils/Seperator.h"
     "${COMPONENT_INCLUDE_DIR}/utils/Singleton.h"
     "${COMPONENT_INCLUDE_DIR}/utils/timer.h"
     "${COMPONENT_INCLUDE_DIR}/utils/utils_hex.h"
     "${COMPONENT_INCLUDE_DIR}/utils/utils_string.h"
     "${COMPONENT_INCLUDE_DIR}/utils/utils_time.h"
     "${COMPONENT_INCLUDE_DIR}/utils/XMLParser.h"
     "${COMPONENT_INCLUDE_DIR}/utils/XMLParserEx.h" )
SET( utils_SOURCE
     "${COMPONENT_SOURCE_DIR}/utils/crc32.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/Deflate.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/DirWalker.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/misc.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/Seperator.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/timer.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/utils_hex.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/utils_string.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/utils_time.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/XMLParser.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${TINYXML_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${ZLIB_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

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

ADD_LIBRARY( "${COMPONENT_NAME}"
             ${INCLUDE}           ${SOURCE}
             ${database_INCLUDE}  ${database_SOURCE}
             ${log_INCLUDE}       ${log_SOURCE}
             ${network_INCLUDE}   ${network_SOURCE}
             ${threading_INCLUDE} ${threading_SOURCE}
             ${utils_INCLUDE}     ${utils_SOURCE} )

TARGET_BUILD_PCH( "${COMPONENT_NAME}"
                  "CommonPCH.h"
                  "common.cpp" )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
