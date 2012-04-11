#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "utils" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )

SET( TARGET_INCLUDE_DIRS
     "${TARGET_INCLUDE_DIR}" )

# Export directories
SET( ${TARGET_NAME}_INCLUDE_DIRS
     ${TARGET_INCLUDE_DIRS}
     PARENT_SCOPE )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/Base64.h"
     "${TARGET_INCLUDE_DIR}/TriFile.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/Base64.cpp"
     "${TARGET_SOURCE_DIR}/TriFile.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE} ${SOURCE} )
