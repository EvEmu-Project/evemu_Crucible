#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-tool" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/EVEToolPCH.h"
     "${TARGET_INCLUDE_DIR}/main.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/Commands.cpp"
     "${TARGET_SOURCE_DIR}/main.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${utils_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_PACKETS_DIR}" )
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "EVEToolPCH.h"
                  "main.cpp" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" "common"
                       "utils"
                       ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES}
                       ${PROJECT_STANDARD_LIBRARIES} )

INSTALL( TARGETS "${TARGET_NAME}"
         RUNTIME DESTINATION "bin" )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
