# - Find MySQL
# Find the native MySQL includes and library
# http://www.cmake.org/Wiki/CMakeUserFindMySQL
# Slightly modified
#
#   MYSQL_FOUND       - True if MySQL found.
#   MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#   MYSQL_LIBRARIES   - List of libraries when using MySQL.
#

IF( MYSQL_INCLUDE_DIR )
    # Already in cache, be silent
    SET( MySQL_FIND_QUIETLY TRUE )
ENDIF( MYSQL_INCLUDE_DIR )

FIND_PATH( MYSQL_INCLUDE_DIR "mysql.h"
           PATH_SUFFIXES "mysql" )

# we need to find the dynamic library here; it seems to be
# mysqlclient on Linux, but libmysql on Windows ...
IF( WIN32 )
    FIND_LIBRARY( MYSQL_LIBRARIES
                  NAMES "libmysql"
                  PATH_SUFFIXES "mysql" )
ELSE( WIN32 )
    FIND_LIBRARY( MYSQL_LIBRARIES
                  NAMES "mysqlclient_r" "mysqlclient"
                  PATH_SUFFIXES "mysql" )
ENDIF( WIN32 )

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "MySQL" DEFAULT_MSG MYSQL_INCLUDE_DIR MYSQL_LIBRARIES )

MARK_AS_ADVANCED( MYSQL_INCLUDE_DIR MYSQL_LIBRARIES )
