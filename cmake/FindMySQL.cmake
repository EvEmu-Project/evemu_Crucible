# - Find MySQL
# Find the native MySQL includes and library
# http://www.cmake.org/Wiki/CMakeUserFindMySQL
# Slightly modified
#
#   MYSQL_FOUND        - True if MySQL found.
#   MYSQL_INCLUDE_DIRS - where to find mysql.h, etc.
#   MYSQL_LIBRARIES    - List of libraries when using MySQL.
#

# we need to find the dynamic library here; it seems to be
# mysqlclient on Linux, but libmysql on Windows ...
IF( WIN32 )
  LIST( APPEND MYSQL_NAMES "libmysql" )
ELSE( WIN32 )
  LIST( APPEND MYSQL_NAMES "mysqlclient_r" "mysqlclient" )
ENDIF( WIN32 )

FIND_PATH(
  MYSQL_INCLUDE_DIRS "mysql.h"
  PATH_SUFFIXES "mysql"
  )
FIND_LIBRARY(
  MYSQL_LIBRARIES
  NAMES ${MYSQL_NAMES}
  )
MARK_AS_ADVANCED(
  MYSQL_INCLUDE_DIRS
  MYSQL_LIBRARIES
  )

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  "MySQL"
  DEFAULT_MSG
  MYSQL_LIBRARIES
  MYSQL_INCLUDE_DIRS
  )
