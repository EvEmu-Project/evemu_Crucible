#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
# Since this is header-only library, there are no sources to be built,
# so we can't use plain ADD_LIBRARY
ADD_CUSTOM_TARGET(
  "asio"
  SOURCES
  "asio.hpp"
  )

SET( ASIO_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PARENT_SCOPE )
SET( ASIO_LIBRARIES
  ""
  PARENT_SCOPE )
