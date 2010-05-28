#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "utf8cpp" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/utf8.h"
     "${COMPONENT_INCLUDE_DIR}/utf8-core.h"
     "${COMPONENT_INCLUDE_DIR}/utf8-checked.h"
     "${COMPONENT_INCLUDE_DIR}/utf8-unchecked.h" )
SET( SOURCE
     "" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

# Since this is header-only library, there are no sources to be built,
# so we can't use plain ADD_LIBRARY
ADD_CUSTOM_TARGET( "${COMPONENT_NAME}"
                   SOURCES ${INCLUDE} ${SOURCE} )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
