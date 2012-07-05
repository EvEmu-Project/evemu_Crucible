#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

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
