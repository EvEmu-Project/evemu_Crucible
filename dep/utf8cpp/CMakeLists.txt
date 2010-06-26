#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "utf8cpp" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/utf8.h"
     "${TARGET_INCLUDE_DIR}/utf8-core.h"
     "${TARGET_INCLUDE_DIR}/utf8-checked.h"
     "${TARGET_INCLUDE_DIR}/utf8-unchecked.h" )
SET( SOURCE
     "" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}" )

# Since this is header-only library, there are no sources to be built,
# so we can't use plain ADD_LIBRARY
ADD_CUSTOM_TARGET( "${TARGET_NAME}"
                   SOURCES ${INCLUDE} ${SOURCE} )

######################
# Export directories #
######################
SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
