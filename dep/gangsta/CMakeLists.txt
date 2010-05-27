#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "gangsta" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/dep/${COMPONENT_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${COMPONENT_INCLUDE_DIR}/GaMath.h"
     "${COMPONENT_INCLUDE_DIR}/GaPreReqs.h"
     "${COMPONENT_INCLUDE_DIR}/GaTypes.h" )
SET( SOURCE
     "${COMPONENT_SOURCE_DIR}/GaMath.cpp"
     "${COMPONENT_SOURCE_DIR}/GaTypes.cpp" )

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
