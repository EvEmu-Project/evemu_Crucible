#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "xmlpktgen" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${COMPONENT_NAME}" )

#########
# Files #
#########
# xmlpktgen
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/Generator.h"
     "${COMPONENT_INCLUDE_DIR}/CloneGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/ConstructGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/DecodeGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/DestructGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/DumpGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/EncodeGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/HeaderGenerator.h"
     "${COMPONENT_INCLUDE_DIR}/XMLPacketGen.h"
     "${COMPONENT_INCLUDE_DIR}/XMLPktGenPCH.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/main.cpp"
     "${COMPONENT_SOURCE_DIR}/Generator.cpp"
     "${COMPONENT_SOURCE_DIR}/CloneGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/ConstructGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/DecodeGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/DestructGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/DumpGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/EncodeGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/HeaderGenerator.cpp"
     "${COMPONENT_SOURCE_DIR}/XMLPacketGen.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${dep_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${COMPONENT_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${COMPONENT_NAME}" "XMLPktGenPCH.h" "main.cpp" )
TARGET_LINK_LIBRARIES( "${COMPONENT_NAME}"
                       "common" "dep" )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
