# - Build config of Boost
# Defines several variables needed for automatic
# download and build of source archive.
#

# We need "boost::asio::transfer_exactly" which is present
# in Boost 1.47.0 and later.
FIND_PACKAGE( "Boost" 1.47.0 COMPONENTS "system" )
SET( PKG_FOUND "${Boost_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( Boost_URL
  "http://downloads.sourceforge.net/project/boost/boost/1.49.0/boost_1_49_0.tar.bz2"
  CACHE STRING "URL of the Boost source archive" )
MARK_AS_ADVANCED( Boost_URL )

# Setup the needed variables
SET( PKG_URL "${Boost_URL}" )
SET( PKG_MD5 "0d202cb811f934282dea64856a175698" )
SET( PKG_ARCHIVE_PREFIX "/boost_1_49_0" )

SET( PKG_CMAKELISTS "
# Disable autolinking of any Boost libraries
ADD_DEFINITIONS( \"-DBOOST_ALL_NO_LIB\" )

INCLUDE_DIRECTORIES(
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  )
ADD_LIBRARY(
  \"boost_system\"
  # Source:
  \"boost/system/api_config.hpp\"
  \"boost/system/config.hpp\"
  \"boost/system/cygwin_error.hpp\"
  \"boost/system/error_code.hpp\"
  \"boost/system/linux_error.hpp\"
  \"boost/system/system_error.hpp\"
  \"boost/system/windows_error.hpp\"
  \"libs/system/src/error_code.cpp\"
  \"libs/system/src/local_free_on_destruction.hpp\"
  )

SET( Boost_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  PARENT_SCOPE )
SET( Boost_LIBRARIES
  \"boost_system\"
  PARENT_SCOPE )
" )
