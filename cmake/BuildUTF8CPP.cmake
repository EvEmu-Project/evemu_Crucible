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
  "https://codeload.github.com/nemtrif/utfcpp/zip/refs/tags/v2.3.4"
  CACHE STRING "URL of the UTF8-CPP source archive" )
MARK_AS_ADVANCED( UTF8CPP_URL )

# Setup the needed variables
SET( PKG_URL "${UTF8CPP_URL}" )
SET( PKG_MD5 "29959274827576EBECC504701928E865" )
SET( PKG_EXTRACT_PREFIX "/utfcpp-2.3.4" )

SET( PKG_CMAKELISTS "
SET( UTF8CPP_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}/utfcpp-2.3.4/source\"
  PARENT_SCOPE )
" )
