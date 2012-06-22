#
# CMake build system file for EVEmu
#
# Author: Bloody.Rabbit
#

FIND_PACKAGE( "TinyXML" QUIET )
IF( TINYXML_FOUND )
  # TinyXML found in system
  RETURN()
ENDIF()

IF( TIXML_USE_STL )
  ADD_DEFINITIONS( "-DTIXML_USE_STL" )
ENDIF( TIXML_USE_STL )

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
ADD_LIBRARY(
  "tinyxml"
  # Source:
  "tinystr.h"
  "tinystr.cpp"
  "tinyxml.h"
  "tinyxml.cpp"
  "tinyxmlerror.cpp"
  "tinyxmlparser.cpp"
  )

SET( TINYXML_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PARENT_SCOPE )
SET( TINYXML_LIBRARIES
  "tinyxml"
  PARENT_SCOPE )
