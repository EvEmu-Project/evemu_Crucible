#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
ADD_LIBRARY(
  "utils"
  # Source:
  "Base64.h"
  "Base64.cpp"
  "TriFile.h"
  "TriFile.cpp"
  )

SET( UTILS_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PARENT_SCOPE )
SET( UTILS_LIBRARIES
  "utils"
  PARENT_SCOPE )
