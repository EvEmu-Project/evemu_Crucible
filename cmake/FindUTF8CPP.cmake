# - Find UTF8CPP
# Find the native UTF8CPP includes
#
#   UTF8CPP_FOUND        - True if UTF8CPP found.
#   UTF8CPP_INCLUDE_DIRS - where to find utf8.h, etc.
#

FIND_PATH(
  UTF8CPP_INCLUDE_DIRS
  "utf8.h"
  )
MARK_AS_ADVANCED(
  UTF8CPP_INCLUDE_DIRS
  )

# handle the QUIETLY and REQUIRED arguments and set UTF8CPP_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  "UTF8CPP"
  DEFAULT_MSG
  UTF8CPP_INCLUDE_DIRS
  )
