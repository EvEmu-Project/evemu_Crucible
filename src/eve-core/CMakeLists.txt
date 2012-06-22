#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

################
# Dependencies #
################
ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/asio"
                  "${PROJECT_BINARY_DIR}/dep/asio" )

FIND_PACKAGE( "Threads" REQUIRED )

FIND_PACKAGE( "Boost"   QUIET )
FIND_PACKAGE( "Gangsta" QUIET )
FIND_PACKAGE( "TinyXML" QUIET )
FIND_PACKAGE( "ZLIB"    QUIET )

IF( NOT Boost_FOUND )
  ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/boost"
                    "${PROJECT_BINARY_DIR}/dep/boost" )

  SET( Boost_FOUND        TRUE )
  SET( Boost_INCLUDE_DIRS ${boost_INCLUDE_DIRS} )
  SET( Boost_LIBRARIES    "boost" )
ENDIF( NOT Boost_FOUND )
MARK_AS_ADVANCED( Boost_DIR Boost_LIB_DIAGNOSTIC_DEFINITIONS )

IF( NOT GANGSTA_FOUND )
  ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/gangsta"
                    "${PROJECT_BINARY_DIR}/dep/gangsta" )

  SET( GANGSTA_FOUND        TRUE )
  SET( GANGSTA_INCLUDE_DIRS ${gangsta_INCLUDE_DIRS} )
  SET( GANGSTA_LIBRARIES    "gangsta" )
ENDIF( NOT GANGSTA_FOUND )

IF( NOT TINYXML_FOUND )
  ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/tinyxml"
                    "${PROJECT_BINARY_DIR}/dep/tinyxml" )

  SET( TINYXML_FOUND        TRUE )
  SET( TINYXML_INCLUDE_DIRS ${tinyxml_INCLUDE_DIRS} )
  SET( TINYXML_LIBRARIES    "tinyxml" )
ENDIF( NOT TINYXML_FOUND )

IF( NOT ZLIB_FOUND )
  ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/zlib"
                    "${PROJECT_BINARY_DIR}/dep/zlib" )

  SET( ZLIB_FOUND        TRUE )
  SET( ZLIB_INCLUDE_DIRS ${zlib_INCLUDE_DIRS} )
  SET( ZLIB_LIBRARIES    "zlib" )
ENDIF( NOT ZLIB_FOUND )

#######################
# Binary dependencies #
#######################
IF( NOT WIN32 )
  FIND_PACKAGE( "MySQL" 5.0 REQUIRED )
ELSE()
  FIND_PACKAGE( "MySQL" 5.0 QUIET )

  IF( NOT MYSQL_FOUND )
    ADD_SUBDIRECTORY( "${PROJECT_SOURCE_DIR}/dep/mysql"
                      "${PROJECT_BINARY_DIR}/dep/mysql" )

    SET( MYSQL_FOUND        TRUE )
    SET( MYSQL_INCLUDE_DIRS ${mysql_INCLUDE_DIRS} )
    SET( MYSQL_LIBRARIES    ${mysql_LIBRARIES} )
  ENDIF( NOT MYSQL_FOUND )
ENDIF( NOT WIN32 )

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-core" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-core.h"
     "${TARGET_INCLUDE_DIR}/eve-compat.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/eve-compat.cpp" )

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
     "${TARGET_INCLUDE_DIR}/utils/FastInt.h"
     "${TARGET_INCLUDE_DIR}/utils/gpoint.h"
     "${TARGET_INCLUDE_DIR}/utils/Lock.h"
     "${TARGET_INCLUDE_DIR}/utils/misc.h"
     "${TARGET_INCLUDE_DIR}/utils/RefPtr.h"
     "${TARGET_INCLUDE_DIR}/utils/SafeMem.h"
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
                  "${TARGET_INCLUDE_DIR}/eve-core.h"
                  "${TARGET_SOURCE_DIR}/eve-compat.cpp" )
TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                            ${Boost_INCLUDE_DIRS}
                            ${MYSQL_INCLUDE_DIRS}
                            ${GANGSTA_INCLUDE_DIRS}
                            ${TINYXML_INCLUDE_DIRS}
                            ${ZLIB_INCLUDE_DIRS}
                            ${asio_INCLUDE_DIRS}
                            "${TARGET_INCLUDE_DIR}" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       ${MYSQL_LIBRARIES}
                       ${GANGSTA_LIBRARIES}
                       ${TINYXML_LIBRARIES}
                       ${ZLIB_LIBRARIES}
                       ${CMAKE_THREAD_LIBS_INIT} )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                         "wsock32" )
ENDIF( WIN32 )
