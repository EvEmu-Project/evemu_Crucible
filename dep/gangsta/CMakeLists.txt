#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

FIND_PACKAGE( "Gangsta" QUIET )
IF( GANGSTA_FOUND )
  # Gangsta found in system
  RETURN()
ENDIF()

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
ADD_LIBRARY(
  "gangsta"
  # Source:
  "GaMath.h"
  "GaMath.cpp"
  "GaPreReqs.h"
  "GaTypes.cpp"
  "GaTypes.h"
  )

SET( GANGSTA_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PARENT_SCOPE )
SET( GANGSTA_LIBRARIES
  "gangsta"
  PARENT_SCOPE )
