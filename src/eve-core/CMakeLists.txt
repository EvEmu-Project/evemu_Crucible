#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-core" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
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

SET( error_INCLUDE
     "${TARGET_INCLUDE_DIR}/error/ErrorHandler.h" )
SET( error_SOURCE
     "${TARGET_SOURCE_DIR}/error/ErrorHandler.cpp" )

SET( log_INCLUDE
     "${TARGET_INCLUDE_DIR}/log/LogNew.h"
     "${TARGET_INCLUDE_DIR}/log/logsys.h"
     "${TARGET_INCLUDE_DIR}/log/logtypes.h" )
SET( log_SOURCE
     "${TARGET_SOURCE_DIR}/log/LogNew.cpp"
     "${TARGET_SOURCE_DIR}/log/logsys.cpp" )

SET( math_INCLUDE
     "${TARGET_INCLUDE_DIR}/math/gpoint.h"
     "${TARGET_INCLUDE_DIR}/math/Trig.h")
     #"${TARGET_INCLUDE_DIR}/math/Vector3D.h")
SET( math_SOURCE
     #"${TARGET_SOURCE_DIR}/math/Vector3D.cpp"
     )

SET( memory_INCLUDE
     "${TARGET_INCLUDE_DIR}/memory/RefPtr.h"
     "${TARGET_INCLUDE_DIR}/memory/SafeMem.h" 
     "${TARGET_INCLUDE_DIR}/memory/Allocator.h" 
     "${TARGET_INCLUDE_DIR}/memory/StackAllocator.h" )
SET( memory_SOURCE
     "${TARGET_SOURCE_DIR}/memory/Allocator.cpp" 
     "${TARGET_SOURCE_DIR}/memory/StackAllocator.cpp" )

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
     "${TARGET_INCLUDE_DIR}/threading/Mutex.h"
     "${TARGET_INCLUDE_DIR}/threading/Threading.h" )
SET( threading_SOURCE
     "${TARGET_SOURCE_DIR}/threading/Mutex.cpp"
     "${TARGET_SOURCE_DIR}/threading/Threading.cpp" )

SET( utils_INCLUDE
     "${TARGET_INCLUDE_DIR}/utils/Buffer.h"
     "${TARGET_INCLUDE_DIR}/utils/crc32.h"
     "${TARGET_INCLUDE_DIR}/utils/Deflate.h"
     "${TARGET_INCLUDE_DIR}/utils/DirWalker.h"
     "${TARGET_INCLUDE_DIR}/utils/FastInt.h"
     "${TARGET_INCLUDE_DIR}/utils/Lock.h"
     "${TARGET_INCLUDE_DIR}/utils/misc.h"
     "${TARGET_INCLUDE_DIR}/utils/Seperator.h"
     "${TARGET_INCLUDE_DIR}/utils/Singleton.h"
     "${TARGET_INCLUDE_DIR}/utils/str2conv.h"
     "${TARGET_INCLUDE_DIR}/utils/timer.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_hex.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_string.h"
     "${TARGET_INCLUDE_DIR}/utils/utils_time.h"
     "${TARGET_INCLUDE_DIR}/utils/XMLParser.h"
     "${TARGET_INCLUDE_DIR}/utils/XMLParserEx.h"
     "${TARGET_INCLUDE_DIR}/utils/CallStack.h" )
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
     "${TARGET_SOURCE_DIR}/utils/XMLParser.cpp"
     "${TARGET_SOURCE_DIR}/utils/CallStack.cpp" )

#####################
# Setup the library #
#####################
SOURCE_GROUP( "src"            FILES ${INCLUDE} )
SOURCE_GROUP( "src\\database"  FILES ${database_INCLUDE} )
#SOURCE_GROUP( "src\\error"     FILES ${error_INCLUDE} )
SOURCE_GROUP( "src\\log"       FILES ${log_INCLUDE} )
SOURCE_GROUP( "src\\math"      FILES ${math_INCLUDE} )
SOURCE_GROUP( "src\\memory"    FILES ${memory_INCLUDE} )
SOURCE_GROUP( "src\\network"   FILES ${network_INCLUDE} )
SOURCE_GROUP( "src\\threading" FILES ${threading_INCLUDE} )
SOURCE_GROUP( "src\\utils"     FILES ${utils_INCLUDE} )

SOURCE_GROUP( "src"            FILES ${SOURCE} )
SOURCE_GROUP( "src\\database"  FILES ${database_SOURCE} )
#SOURCE_GROUP( "src\\error"     FILES ${error_SOURCE} )
SOURCE_GROUP( "src\\log"       FILES ${log_SOURCE} )
SOURCE_GROUP( "src\\math"      FILES ${math_SOURCE} )
SOURCE_GROUP( "src\\memory"    FILES ${memory_SOURCE} )
SOURCE_GROUP( "src\\network"   FILES ${network_SOURCE} )
SOURCE_GROUP( "src\\threading" FILES ${threading_SOURCE} )
SOURCE_GROUP( "src\\utils"     FILES ${utils_SOURCE} )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE}           ${SOURCE}
             ${database_INCLUDE}  ${database_SOURCE}
             #${error_INCLUDE}     ${error_SOURCE}
             ${log_INCLUDE}       ${log_SOURCE}
             ${math_INCLUDE}      ${math_SOURCE}
             ${memory_INCLUDE}    ${memory_SOURCE}
             ${network_INCLUDE}   ${network_SOURCE}
             ${threading_INCLUDE} ${threading_SOURCE}
             ${utils_INCLUDE}     ${utils_SOURCE} )

# macOS specific header handling
IF(APPLE)
    # Remove problematic flags that cause header conflicts
    STRING(REPLACE "-march=native" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    
    # Add macOS specific workarounds for header conflicts
    ADD_DEFINITIONS(-D__restrict=restrict)
    ADD_DEFINITIONS(-D__DARWIN_C_LEVEL=199506L)
    
    # Additional macOS-specific settings
    # Fix for missing clock_gettime on older macOS versions
    IF(NOT CMAKE_CXX_STANDARD)
        SET(CMAKE_CXX_STANDARD 11)  # Ensure C++11 or higher
    ENDIF()
    
    # Link against macOS frameworks that might be needed
    FIND_LIBRARY(CORESERVICES_LIBRARY CoreServices)
    FIND_LIBRARY(SYSTEMCONFIGURATION_LIBRARY SystemConfiguration)
    IF(CORESERVICES_LIBRARY)
        TARGET_LINK_LIBRARIES("${TARGET_NAME}" ${CORESERVICES_LIBRARY})
    ENDIF()
    IF(SYSTEMCONFIGURATION_LIBRARY)
        TARGET_LINK_LIBRARIES("${TARGET_NAME}" ${SYSTEMCONFIGURATION_LIBRARY})
    ENDIF()
ENDIF()

target_precompile_headers( "${TARGET_NAME}" PUBLIC
                  "${TARGET_INCLUDE_DIR}/eve-core.h" )
TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                            ${Boost_INCLUDE_DIRS}
                            ${MYSQL_INCLUDE_DIRS}
                            ${GANGSTA_INCLUDE_DIRS}
                            ${TINYXML_INCLUDE_DIRS}
                            "${ZLIB_INCLUDE_DIR}"
                            "${TARGET_INCLUDE_DIR}" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       ${Boost_LIBRARIES}
                       ${MYSQL_LIBRARIES}
                       ${GANGSTA_LIBRARIES}
                       ${TINYXML_LIBRARIES}
                       "${ZLIB_LIBRARY}"
                       ${CMAKE_NET_LIBS}
                       ${CMAKE_THREAD_LIBS} )
# Modify the precompiled header creation to avoid conflicting includes
TARGET_BUILD_PCH( eve-core
    "eve-core.h"
    "cmake_pch.hxx"
    CXX
    DISABLE_INCLUDES
    )
    