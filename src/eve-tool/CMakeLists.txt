#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-tool" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-tool.h"
     "${TARGET_INCLUDE_DIR}/Commands.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/eve-tool.cpp"
     "${TARGET_SOURCE_DIR}/Commands.cpp" )

########################
# Setup the executable #
########################
SOURCE_GROUP( "src" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "${TARGET_INCLUDE_DIR}/eve-tool.h"
                  "${TARGET_SOURCE_DIR}/eve-tool.cpp" )
TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                            ${eve-common_INCLUDE_DIRS}
                            "${TARGET_INCLUDE_DIR}" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" )

INSTALL( TARGETS "${TARGET_NAME}"
         RUNTIME DESTINATION "bin" )
