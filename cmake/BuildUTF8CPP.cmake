# - Build config of UTF8-CPP
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "UTF8CPP" )
SET( PKG_FOUND "${UTF8CPP_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( UTF8CPP_URL
  "http://downloads.sourceforge.net/project/utfcpp/utf8cpp_2x/Release%202.3.2/utf8_v2_3_2.zip"
  CACHE STRING "URL of the UTF8-CPP source archive" )
MARK_AS_ADVANCED( UTF8CPP_URL )

# Setup the needed variables
SET( PKG_URL "${UTF8CPP_URL}" )
SET( PKG_MD5 "e01080a941dfda1136b992d996245e77" )
SET( PKG_EXTRACT_PREFIX "/utf8_v2_3_2" )

SET( PKG_CMAKELISTS "
SET( UTF8CPP_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}/source\"
  PARENT_SCOPE )
" )
