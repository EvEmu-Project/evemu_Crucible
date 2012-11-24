#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-xmlpktgen" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-xmlpktgen.h"
     "${TARGET_INCLUDE_DIR}/Generator.h"
     "${TARGET_INCLUDE_DIR}/CloneGenerator.h"
     "${TARGET_INCLUDE_DIR}/ConstructGenerator.h"
     "${TARGET_INCLUDE_DIR}/DecodeGenerator.h"
     "${TARGET_INCLUDE_DIR}/DestructGenerator.h"
     "${TARGET_INCLUDE_DIR}/DumpGenerator.h"
     "${TARGET_INCLUDE_DIR}/EncodeGenerator.h"
     "${TARGET_INCLUDE_DIR}/HeaderGenerator.h"
     "${TARGET_INCLUDE_DIR}/XMLPacketGen.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/eve-xmlpktgen.cpp"
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
SOURCE_GROUP( "src" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

IF( NOT EVEMU_TARGETS_IMPORT )
  ADD_EXECUTABLE( "${TARGET_NAME}"
                  ${INCLUDE} ${SOURCE} )

  TARGET_BUILD_PCH( "${TARGET_NAME}"
                    "${TARGET_INCLUDE_DIR}/eve-xmlpktgen.h"
                    "${TARGET_SOURCE_DIR}/eve-xmlpktgen.cpp" )
  TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                              ${eve-core_INCLUDE_DIRS}
                              "${TARGET_INCLUDE_DIR}" )
  TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                         "eve-core" )

  IF( EVEMU_TARGETS_EXPORT )
    EXPORT( TARGETS "${TARGET_NAME}"
            FILE "${EVEMU_TARGETS_EXPORT}" )
  ENDIF( EVEMU_TARGETS_EXPORT )
ENDIF( NOT EVEMU_TARGETS_IMPORT )
