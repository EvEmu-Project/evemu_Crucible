#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "zlib" )
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
     "${TARGET_INCLUDE_DIR}/crc32.h"
     "${TARGET_INCLUDE_DIR}/deflate.h"
     "${TARGET_INCLUDE_DIR}/gzguts.h"
     "${TARGET_INCLUDE_DIR}/inffast.h"
     "${TARGET_INCLUDE_DIR}/inffixed.h"
     "${TARGET_INCLUDE_DIR}/inflate.h"
     "${TARGET_INCLUDE_DIR}/inftrees.h"
     "${TARGET_INCLUDE_DIR}/trees.h"
     "${TARGET_INCLUDE_DIR}/zconf.h"
     "${TARGET_INCLUDE_DIR}/zlib.h"
     "${TARGET_INCLUDE_DIR}/zutil.h" )

SET( SOURCE
     "${TARGET_SOURCE_DIR}/adler32.c"
     "${TARGET_SOURCE_DIR}/compress.c"
     "${TARGET_SOURCE_DIR}/crc32.c"
     "${TARGET_SOURCE_DIR}/deflate.c"
     "${TARGET_SOURCE_DIR}/gzclose.c"
     "${TARGET_SOURCE_DIR}/gzlib.c"
     "${TARGET_SOURCE_DIR}/gzread.c"
     "${TARGET_SOURCE_DIR}/gzwrite.c"
     "${TARGET_SOURCE_DIR}/infback.c"
     "${TARGET_SOURCE_DIR}/inffast.c"
     "${TARGET_SOURCE_DIR}/inflate.c"
     "${TARGET_SOURCE_DIR}/inftrees.c"
     "${TARGET_SOURCE_DIR}/trees.c"
     "${TARGET_SOURCE_DIR}/uncompr.c"
     "${TARGET_SOURCE_DIR}/zutil.c" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

ADD_LIBRARY( "${TARGET_NAME}"
             ${INCLUDE} ${SOURCE} )
