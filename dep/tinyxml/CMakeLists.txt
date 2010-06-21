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
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE} ${SOURCE} )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
