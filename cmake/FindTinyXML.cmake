# - Find TinyXML
# Find the native TinyXML includes and library
#
#   TINYXML_FOUND        - True if TinyXML found.
#   TINYXML_INCLUDE_DIRS - where to find tinyxml.h, etc.
#   TINYXML_LIBRARIES    - List of libraries when using TinyXML.
#

LIST( APPEND TINYXML_NAMES "tinyxml" )

FIND_PATH(
  TINYXML_INCLUDE_DIRS
  "tinyxml.h"
  )
FIND_LIBRARY(
  TINYXML_LIBRARIES
  NAMES "tinyxml"
  )
MARK_AS_ADVANCED(
  TINYXML_INCLUDE_DIRS
  TINYXML_LIBRARIES
  )

# handle the QUIETLY and REQUIRED arguments and set TINYXML_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  "TinyXML"
  DEFAULT_MSG
  TINYXML_INCLUDE_DIRS
  TINYXML_LIBRARIES
  )
