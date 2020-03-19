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
    SET( PKG_MD5 "8157cafab6f57418b441c228f6db3992" )
#    # Windows 64-bit
#    SET( PKG_NAME "mysql-noinstall-5.1.63-winx64" )
#    SET( PKG_MD5 "ae0289b7788666b1254d14b001ff6ba9" )
  ELSE()
    # Windows 32-bit
    SET( PKG_NAME "mysql-5.5.25a-win32" )
    SET( PKG_MD5 "df745c7bb9d34cac29503a4e36e3e8c8" )
    #SET( PKG_MD5 "859a538879d9f8ed06dcdcec9475aa78" )
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
	SET( PKG_MD5 "261aa316160c65435a7359cc57561014")
  ELSE()
	# MAC OS-X 32-bit
	SET( PKG_NAME "mysql-5.5.25a-osx10.6-x86" )
	SET( PKG_MD5 "56f19c779a567ef27b82a2df8252facc")
  ENDIF()
  # MAC OS-X
  SET( PKG-ARCHIVE "${PKG_NAME}.tar.gz" )
  SET( PKG-IMPLIB "libmysqlclient_r.a" )
  SET( PKG-LIBRARY "libmysqlclient_r.dylib" )

ELSE ( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Linux 64-bit
    SET( PKG_NAME "mysql-5.5.25a-linux2.6-x86_64" )
    SET( PKG_MD5 "217f97e79d68d931a4790790ea8a8894" )
#    # Linux 64-bit
#    SET( PKG_NAME "mysql-5.1.63-linux-x86_64-glibc23" )
#    SET( PKG_MD5 "594ea37fcd9f29a9e3eddf38e7288e3f" )
  ELSE()
    # Linux 32-bit
    SET( PKG_NAME "mysql-5.5.25a-linux2.6-i686" )
    SET( PKG_MD5 "ff5d742bfff4a0b8ef48da336f13e6a6" )
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
#  "https://downloads.skysql.com/archives/mysql-5.5/${PKG_ARCHIVE}"
# "ftp://ftp.fu-berlin.de/unix/databases/mysql/Downloads/MySQL-5.5/${PKG_ARCHIVE}"
# "https://downloads.mysql.com/archives/get/file/${PKG_ARCHIVE}"
 "https://downloads.mysql.com/archives/get/p/23/file/${PKG_ARCHIVE}"
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
