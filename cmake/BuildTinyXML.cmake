# - Build config of TinyXML
# Defines several variables needed for automatic
# download and build of source archive.
#

# We need "TiXmlNode::TINYXML_ELEMENT" which is present
# in TinyXML 2.6.0 and later.
FIND_PACKAGE( "TinyXML" 2.6.0 )
SET( PKG_FOUND "${TINYXML_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( TINYXML_URL
  "http://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.tar.gz"
  CACHE STRING "URL of the TinyXML source archive" )
MARK_AS_ADVANCED( TINYXML_URL )

# Setup the needed variables
SET( PKG_URL "${TINYXML_URL}" )
SET( PKG_MD5 "c1b864c96804a10526540c664ade67f0" )
SET( PKG_ARCHIVE_PREFIX "/tinyxml" )

SET( PKG_CMAKELISTS "
IF( TIXML_USE_STL )
  ADD_DEFINITIONS( \"-DTIXML_USE_STL\" )
ENDIF( TIXML_USE_STL )

INCLUDE_DIRECTORIES(
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  )
ADD_LIBRARY(
  \"tinyxml\"
  # Source:
  \"tinystr.h\"
  \"tinystr.cpp\"
  \"tinyxml.h\"
  \"tinyxml.cpp\"
  \"tinyxmlerror.cpp\"
  \"tinyxmlparser.cpp\"
  )

SET( TINYXML_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  PARENT_SCOPE )
SET( TINYXML_LIBRARIES
  \"tinyxml\"
  PARENT_SCOPE )
" )
