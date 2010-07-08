#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "xmlpktgen" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/Generator.h"
     "${TARGET_INCLUDE_DIR}/CloneGenerator.h"
     "${TARGET_INCLUDE_DIR}/ConstructGenerator.h"
     "${TARGET_INCLUDE_DIR}/DecodeGenerator.h"
     "${TARGET_INCLUDE_DIR}/DestructGenerator.h"
     "${TARGET_INCLUDE_DIR}/DumpGenerator.h"
     "${TARGET_INCLUDE_DIR}/EncodeGenerator.h"
     "${TARGET_INCLUDE_DIR}/HeaderGenerator.h"
     "${TARGET_INCLUDE_DIR}/XMLPacketGen.h"
     "${TARGET_INCLUDE_DIR}/XMLPktGenPCH.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/main.cpp"
     "${TARGET_SOURCE_DIR}/Generator.cpp"
     "${TARGET_SOURCE_DIR}/CloneGenerator.cpp"
     "${TARGET_SOURCE_DIR}/ConstructGenerator.cpp"
     "${TARGET_SOURCE_DIR}/DecodeGenerator.cpp"
     "${TARGET_SOURCE_DIR}/DestructGenerator.cpp"
     "${TARGET_SOURCE_DIR}/DumpGenerator.cpp"
     "${TARGET_SOURCE_DIR}/EncodeGenerator.cpp"
     "${TARGET_SOURCE_DIR}/HeaderGenerator.cpp"
     "${TARGET_SOURCE_DIR}/XMLPacketGen.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${TINYXML_INCLUDE_DIR}" )

INCLUDE_DIRECTORIES( "${common_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "XMLPktGenPCH.h"
                  "main.cpp" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "common"
                       ${TINYXML_LIBRARIES}
                       ${PROJECT_STANDARD_LIBRARIES} )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
