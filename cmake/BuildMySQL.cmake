# - Build config of MySQL
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "MySQL" 5.0 QUIET )
SET( PKG_FOUND "${MYSQL_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Choose correct platform-dependent properties
IF( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Windows 64-bit
    SET( PKG_NAME "mysql-5.5.25-winx64" )
    SET( PKG_MD5 "322b459c5bbda38ed93aed3db2b60d08" )
  ELSE()
    # Windows 32-bit
    SET( PKG_NAME "mysql-5.5.25-win32" )
    SET( PKG_MD5 "e4cfff7e06d8c980217f44ea7659a8a2" )
  ENDIF()

  # Windows
  SET( PKG_ARCHIVE "${PKG_NAME}.zip" )
  SET( PKG_IMPLIB "libmysql.lib")
  SET( PKG_LIBRARY "libmysql.dll" )
ELSE( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Linux 64-bit
    SET( PKG_NAME "mysql-5.5.25-linux2.6-x86_64" )
    SET( PKG_MD5 "a6ab7957d4e6bbdc253ff1c99901f8b3" )
  ELSE()
    # Linux 32-bit
    SET( PKG_NAME "mysql-5.5.25-linux2.6-i686" )
    SET( PKG_MD5 "0fc793ef21b8ce5817bc276233357d2b" )
  ENDIF()

  # Linux
  SET( PKG_ARCHIVE "${PKG_NAME}.tar.gz" )
  SET( PKG_IMPLIB "libmysqlclient_r.a" )
  SET( PKG_LIBRARY "libmysqlclient_r.so" )
ENDIF( WIN32 )

# Cache entries
SET( MYSQL_URL
  "http://cdn.mysql.com/Downloads/MySQL-5.5/${PKG_ARCHIVE}"
  CACHE STRING "URL of the MySQL source archive" )
MARK_AS_ADVANCED( MYSQL_URL )

# Setup the needed variables
SET( PKG_URL "${MYSQL_URL}" )
SET( PKG_ARCHIVE_PREFIX "/${PKG_NAME}" )

SET( PKG_CMAKELISTS "
ADD_LIBRARY(
  \"mysql\" SHARED
  IMPORTED GLOBAL
  )
SET_TARGET_PROPERTIES(
  \"mysql\" PROPERTIES
  IMPORTED_IMPLIB \"\${CMAKE_CURRENT_SOURCE_DIR}/lib/${PKG_IMPLIB}\"
  IMPORTED_LOCATION \"\${CMAKE_CURRENT_SOURCE_DIR}/lib/${PKG_LIBRARY}\"
  )

SET( MYSQL_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}/include\"
  PARENT_SCOPE )
SET( MYSQL_LIBRARIES
  \"mysql\"
  PARENT_SCOPE )
" )
