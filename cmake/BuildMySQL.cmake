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
    SET( PKG_NAME "mariadb-10.5.9-winx64" )
    SET( PKG_MD5 "2e1252fb008c2697471b16b0b8b4d601" )
    SET( MYSQL_DOWNLOAD_URL "https://downloads.mariadb.com/MariaDB/mariadb-10.5.9/winx64-packages/mariadb-10.5.9-winx64.zip")
  ELSE()
    # Windows 32-bit
    SET( PKG_NAME "mariadb-10.5.9-win32" )
    SET( PKG_MD5 "96ae3f43ffa9586a083ce08b858eb0c4" )
    SET( MYSQL_DOWNLOAD_URL "https://downloads.mariadb.com/MariaDB/mariadb-10.5.9/win32-packages/mariadb-10.5.9-win32.zip")
  ENDIF()

  # Windows
  SET( PKG_ARCHIVE "${PKG_NAME}.zip" )
  SET( PKG_IMPLIB "libmariadb.lib")
  SET( PKG_LIBRARY "libmariadb.dll" )

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
  SET( MYSQL_DOWNLOAD_URL "https://downloads.mysql.com/archives/get/p/23/file/${PKG_ARCHIVE}")
ELSE ( WIN32 )
  IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # Linux 64-bit
    SET( PKG_NAME "mariadb-10.5.9-linux-x86_64" )
    SET( PKG_MD5 "0f94e0f2a925a4eb2e334f6b188cfbb9" )
    SET( MYSQL_DOWNLOAD_URL "https://downloads.mariadb.com/MariaDB/mariadb-10.5.9/bintar-linux-x86_64/mariadb-10.5.9-linux-x86_64.tar.gz")
  ELSE()
    # Linux 32-bit
    SET( PKG_NAME "mariadb-10.5.9-linux-i686" )
    SET( PKG_MD5 "6821fd0f565261a9b9c8ba10f6999d14" )
    SET( MYSQL_DOWNLOAD_URL "https://downloads.mariadb.com/MariaDB/mariadb-10.5.9/bintar-linux-x86/mariadb-10.5.9-linux-i686.tar.gz")
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
   "${MYSQL_DOWNLOAD_URL}"
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
  \"\${CMAKE_CURRENT_SOURCE_DIR}/include/mysql\"
  PARENT_SCOPE )
SET( MYSQL_LIBRARIES
  \"mysql\"
  PARENT_SCOPE )
" )
