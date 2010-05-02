# - Find MySQL
# Find the native MySQL includes and library
# http://www.cmake.org/Wiki/CMakeUserFindMySQL
# Slightly modified
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

IF( MYSQL_INCLUDE_DIR )
  # Already in cache, be silent
  SET( MySql_FIND_QUIETLY TRUE )
ENDIF( MYSQL_INCLUDE_DIR )

FIND_PATH( MYSQL_INCLUDE_DIR mysql.h
           PATH_SUFFIXES mysql )

SET( MYSQL_NAMES mysqlclient mysqlclient_r )
FIND_LIBRARY( MYSQL_LIBRARIES NAMES ${MYSQL_NAMES}
              PATH_SUFFIXES mysql )

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MySql  DEFAULT_MSG  MYSQL_LIBRARIES  MYSQL_INCLUDE_DIR )

MARK_AS_ADVANCED( MYSQL_LIBRARIES MYSQL_INCLUDE_DIR )
