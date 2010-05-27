#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "tinyxml" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/tinystr.h"
     "${COMPONENT_INCLUDE_DIR}/tinyxml.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/tinystr.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxml.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxmlerror.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxmlparser.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

ADD_LIBRARY( "${COMPONENT_NAME}"
             ${INCLUDE} ${SOURCE} )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
