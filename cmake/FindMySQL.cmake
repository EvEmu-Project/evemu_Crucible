# - Find MySQL
# Find the native MySQL includes and library
# http://www.cmake.org/Wiki/CMakeUserFindMySQL
# Slightly modified
#
#  MYSQL_FOUND       - True if MySQL found.
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#

IF( MYSQL_INCLUDE_DIR )
  # Already in cache, be silent
  SET( MySQL_FIND_QUIETLY TRUE )
ENDIF( MYSQL_INCLUDE_DIR )

FIND_PATH( MYSQL_INCLUDE_DIR mysql.h
           PATH_SUFFIXES mysql )

FIND_LIBRARY( MYSQL_LIBRARIES
              NAMES mysqlclient mysqlclient_r
              PATH_SUFFIXES mysql )

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MySQL DEFAULT_MSG MYSQL_INCLUDE_DIR MYSQL_LIBRARIES )

MARK_AS_ADVANCED( MYSQL_INCLUDE_DIR MYSQL_LIBRARIES )
