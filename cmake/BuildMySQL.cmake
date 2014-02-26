# - Build config of MySQL
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "MySQL" 5.0 )
SET( PKG_FOUND "${MYSQL_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Choose correct platform-dependent properties
IF( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Windows 64-bit
    SET( PKG_NAME "mysql-5.5.25a-winx64" )
    SET( PKG_MD5 "80ad4487e09b9b6967c48188aafa888e" )
#    # Windows 64-bit
#    SET( PKG_NAME "mysql-noinstall-5.1.63-winx64" )
#    SET( PKG_MD5 "ae0289b7788666b1254d14b001ff6ba9" )
  ELSE()
    # Windows 32-bit
    SET( PKG_NAME "mysql-5.5.25a-win32" )
    SET( PKG_MD5 "859a538879d9f8ed06dcdcec9475aa78" )
#    # Windows 32-bit
#    SET( PKG_NAME "mysql-5.1.63" )
#    SET( PKG_MD5 "606c3edc0bba77a033e0b2bf134306d9" )
  ENDIF()

  # Windows
  SET( PKG_ARCHIVE "${PKG_NAME}.zip" )
  SET( PKG_IMPLIB "libmysql.lib")
  SET( PKG_LIBRARY "libmysql.dll" )

ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	# MAC OS-X 64-bit
  	SET( PKG_NAME "mysql-5.5.25a-osx10.6-x86_64" )
	SET( PKG_MD5 "6e3c37db5f0b0f7b239790d854c75b70")
  ELSE()
	# MAC OS-X 32-bit
	SET( PKG_NAME "mysql-5.5.25a-osx10.6-x86" )
	SET( PKG_MD5 "c73df15ab9489053890eb02ead2b3d17")
  ENDIF()
  # MAC OS-X
  SET( PKG-ARCHIVE "${PKG_NAME}.tar.gz" )
  SET( PKG-IMPLIB "libmysqlclient_r.a" )
  SET( PKG-LIBRARY "libmysqlclient_r.dylib" )

ELSE ( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Linux 64-bit
    SET( PKG_NAME "mysql-5.5.25a-linux2.6-x86_64" )
    SET( PKG_MD5 "15f29a6eb9bbd3f03a74aa524e7f1531" )
#    # Linux 64-bit
#    SET( PKG_NAME "mysql-5.1.63-linux-x86_64-glibc23" )
#    SET( PKG_MD5 "594ea37fcd9f29a9e3eddf38e7288e3f" )
  ELSE()
    # Linux 32-bit
    SET( PKG_NAME "mysql-5.5.25a-linux2.6-i686" )
    SET( PKG_MD5 "1f054641e48414a28a9adb1c6446e475" )
#    # Linux 32-bit
#    SET( PKG_NAME "mysql-5.1.63-linux-i686-glibc23" )
#    SET( PKG_MD5 "c3a8581320fdd7d11946456d6b3e9d7b" )
  ENDIF()

  # Linux
  SET( PKG_ARCHIVE "${PKG_NAME}.tar.gz" )
  SET( PKG_IMPLIB "libmysqlclient_r.a" )
  SET( PKG_LIBRARY "libmysqlclient_r.so" )
ENDIF( WIN32 )

# Cache entries
SET( MYSQL_URL
#  "http://downloads.mysql.com/archives/mysql-5.1/${PKG_ARCHIVE}"
#  "http://downloads.mysql.com/archives/mysql-5.5/${PKG_ARCHIVE}"
   "https://downloads.skysql.com/archives/mysql-5.5/${PKG_ARCHIVE}"
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
