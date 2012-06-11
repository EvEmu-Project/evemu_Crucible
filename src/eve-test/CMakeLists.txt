#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-test" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

SET( TARGET_INCLUDE_DIRS
     ${eve-common_INCLUDE_DIRS}
     "${TARGET_INCLUDE_DIR}" )

# Export directories
SET( ${TARGET_NAME}_INCLUDE_DIRS
     ${TARGET_INCLUDE_DIRS}
     PARENT_SCOPE )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-test.h" )
# No eve-test.cpp, generated on the fly.

# You must NOT use TARGET_SOURCE_DIR (or, to be
# exact, use absolute paths) when specifying
# the test sources.
SET( utils_SOURCE
     "utils/EvilNumberTest.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

SOURCE_GROUP( "include"    ${INCLUDE} )
SOURCE_GROUP( "src\\utils" ${utils_SOURCE} )

CREATE_TEST_SOURCELIST( TARGET_SOURCELIST "eve-test.cpp"
                        ${utils_SOURCE}
                        EXTRA_INCLUDE "eve-test.h" )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${TARGET_SOURCELIST} )
TARGET_BUILD_PCH( "${TARGET_NAME}"
                  "${TARGET_INCLUDE_DIR}/eve-test.h"
                  "${TARGET_BINARY_DIR}/eve-test.cpp" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" )

#########
# Tests #
#########
ADD_TEST( NAME "EvilNumberTest"
          COMMAND "${TARGET_NAME}" "utils/EvilNumberTest" )
