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

SET( TARGET_INCLUDE_DIRS
     ${eve-common_INCLUDE_DIRS}
     "${TARGET_INCLUDE_DIR}" )

# Export directories
SET( ${TARGET_NAME}_INCLUDE_DIRS
     ${TARGET_INCLUDE_DIRS}
     PARENT_SCOPE )

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
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "${TARGET_INCLUDE_DIR}/EVEToolPCH.h"
                  "${TARGET_SOURCE_DIR}/main.cpp" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" )

INSTALL( TARGETS "${TARGET_NAME}"
         RUNTIME DESTINATION "bin" )
