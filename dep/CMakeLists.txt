#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( COMPONENT_NAME        "dep" )
SET( COMPONENT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${COMPONENT_NAME}" )
SET( COMPONENT_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/${COMPONENT_NAME}" )

#########
# Files #
#########
# dep/gangsta
SET( gangsta_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/gangsta/GaMath.h"
     "${COMPONENT_INCLUDE_DIR}/gangsta/GaPreReqs.h"
     "${COMPONENT_INCLUDE_DIR}/gangsta/GaTypes.h" )
SET( gangsta_SOURCE
     "${COMPONENT_SOURCE_DIR}/gangsta/GaMath.cpp"
     "${COMPONENT_SOURCE_DIR}/gangsta/GaTypes.cpp" )

# dep/tinyxml
SET( tinyxml_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/tinyxml/tinystr.h"
     "${COMPONENT_INCLUDE_DIR}/tinyxml/tinyxml.h" )
SET( tinyxml_SOURCE
     "${COMPONENT_SOURCE_DIR}/tinyxml/tinystr.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxml/tinyxml.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxml/tinyxmlerror.cpp"
     "${COMPONENT_SOURCE_DIR}/tinyxml/tinyxmlparser.cpp" )

# dep/utf8cpp
SET( utf8cpp_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/utf8cpp/utf8.h"
     "${COMPONENT_INCLUDE_DIR}/utf8cpp/utf8-core.h"
     "${COMPONENT_INCLUDE_DIR}/utf8cpp/utf8-checked.h"
     "${COMPONENT_INCLUDE_DIR}/utf8cpp/utf8-unchecked.h" )
SET( utf8cpp_SOURCE
     "" )

# dep/utils
SET( utils_INCLUDE
     "${COMPONENT_INCLUDE_DIR}/utils/Base64.h"
     "${COMPONENT_INCLUDE_DIR}/utils/TriFile.h" )
SET( utils_SOURCE
     "${COMPONENT_SOURCE_DIR}/utils/Base64.cpp"
     "${COMPONENT_SOURCE_DIR}/utils/TriFile.cpp" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( "${COMPONENT_INCLUDE_DIR}" )

SOURCE_GROUP( "gangsta" FILES ${gangsta_INCLUDE} ${gangsta_SOURCE} )
SOURCE_GROUP( "tinyxml" FILES ${tinyxml_INCLUDE} ${tinyxml_SOURCE} )
SOURCE_GROUP( "utf8cpp" FILES ${utf8cpp_INCLUDE} ${utf8cpp_SOURCE} )
SOURCE_GROUP( "utils"   FILES ${utils_INCLUDE}   ${utils_SOURCE} )

ADD_LIBRARY( "${COMPONENT_NAME}"
             ${gangsta_INCLUDE} ${gangsta_SOURCE}
             ${tinyxml_INCLUDE} ${tinyxml_SOURCE}
             ${utf8cpp_INCLUDE} ${utf8cpp_SOURCE}
             ${utils_INCLUDE}   ${utils_SOURCE} )

######################
# Export directories #
######################
SET( ${COMPONENT_NAME}_INCLUDE_DIR "${COMPONENT_INCLUDE_DIR}" PARENT_SCOPE )
