# - Build config of Boost
# Defines several variables needed for automatic
# download and build of source archive.
#

# Boost 1.70 and later install a CMake package config (BoostConfig.cmake), and CMake 3.30 and
# later no longer provide FindBoost (policy CMP0167), so look for the package config first.
# Boost.System is header-only since 1.69 (1.89 dropped its stub library and "system" component
# altogether), so only the headers are needed from an installed Boost.
FIND_PACKAGE( "Boost" 1.69.0 CONFIG QUIET )
IF( Boost_FOUND AND TARGET "Boost::headers" )
  GET_TARGET_PROPERTY( Boost_INCLUDE_DIRS "Boost::headers" INTERFACE_INCLUDE_DIRECTORIES )
  SET( Boost_LIBRARIES "Boost::headers" )
  SET( PKG_FOUND TRUE )
  RETURN()
ENDIF()

# Older CMake still has FindBoost, which also finds Boost releases without a package config.
IF( NOT POLICY CMP0167 )
  FIND_PACKAGE( "Boost" 1.67.0 COMPONENTS "system" )
  SET( PKG_FOUND "${Boost_FOUND}" )

  IF( PKG_FOUND )
    RETURN()
  ENDIF( PKG_FOUND )
ENDIF()

SET( PKG_FOUND FALSE )

# Cache entries
SET( Boost_URL
  "https://archives.boost.io/release/1.67.0/source/boost_1_67_0.tar.bz2"
  CACHE STRING "URL of the Boost source archive" )
MARK_AS_ADVANCED( Boost_URL )

# Setup the needed variables
SET( PKG_URL "${Boost_URL}" )
SET( PKG_MD5 "ced776cb19428ab8488774e1415535ab" )
SET( PKG_ARCHIVE_PREFIX "/boost_1_67_0" )

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
  )

SET( Boost_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  PARENT_SCOPE )
SET( Boost_LIBRARIES
  \"boost_system\"
  PARENT_SCOPE )
" )
