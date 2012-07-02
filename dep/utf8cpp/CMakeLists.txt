#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

FIND_PACKAGE( "UTF8CPP" QUIET )
IF( UTF8CPP_FOUND )
  # UTF8CPP found in system
  RETURN()
ENDIF()

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
# Since this is header-only library, there are no sources to be built,
# so we can't use plain ADD_LIBRARY
ADD_CUSTOM_TARGET(
  "utf8cpp"
  SOURCES
  "utf8.h"
  "utf8-core.h"
  "utf8-checked.h"
  "utf8-unchecked.h"
  )

SET( UTF8CPP_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PARENT_SCOPE )
SET( UTF8CPP_LIBRARIES
  ""
  PARENT_SCOPE )
