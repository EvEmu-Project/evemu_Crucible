#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "tinyxml" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )

SET( TARGET_INCLUDE_DIRS
     "${TARGET_INCLUDE_DIR}" )

# Export directories
SET( ${TARGET_NAME}_INCLUDE_DIRS
     ${TARGET_INCLUDE_DIRS}
     PARENT_SCOPE )

#################
# Configuration #
#################
SET( TIXML_USE_STL ON
     CACHE BOOL "tinyxml will use native STL." )
MARK_AS_ADVANCED( TIXML_USE_STL )

IF( TIXML_USE_STL )
  ADD_DEFINITIONS( "-DTIXML_USE_STL=1" )
ENDIF( TIXML_USE_STL )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/tinystr.h"
     "${TARGET_INCLUDE_DIR}/tinyxml.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/tinystr.cpp"
     "${TARGET_SOURCE_DIR}/tinyxml.cpp"
     "${TARGET_SOURCE_DIR}/tinyxmlerror.cpp"
     "${TARGET_SOURCE_DIR}/tinyxmlparser.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

SOURCE_GROUP( "include" INCLUDE )
SOURCE_GROUP( "src"     SOURCE )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE} ${SOURCE} )
