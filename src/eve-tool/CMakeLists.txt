#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "eve-tool" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${COMPONENT_NAME}" )

#########
# Files #
#########
# eve-tool
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/EVEToolPCH.h"
     "${COMPONENT_INCLUDE_DIR}/main.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/Commands.cpp"
     "${COMPONENT_SOURCE_DIR}/main.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${MYSQL_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${dep_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${eve-common_PACKETS_DIR}" )
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${COMPONENT_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${COMPONENT_NAME}" "EVEToolPCH.h" "main.cpp" )
TARGET_LINK_LIBRARIES( "${COMPONENT_NAME}"
                       "eve-common" "common" "dep"
                       ${MYSQL_LIBRARIES} ${ZLIB_LIBRARIES} )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
