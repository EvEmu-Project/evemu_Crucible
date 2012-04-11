# - Find Gangsta
# Find the native Gangsta includes and library
#
#   GANGSTA_FOUND        - True if Gangsta found.
#   GANGSTA_INCLUDE_DIRS - where to find GaTypes.h, etc.
#   GANGSTA_LIBRARIES    - List of libraries when using Gangsta.
#

IF( GANGSTA_INCLUDE_DIRS )
    # Already in cache, be silent
    SET( Gangsta_FIND_QUIETLY TRUE )
ENDIF( GANGSTA_INCLUDE_DIRS )

FIND_PATH( GANGSTA_INCLUDE_DIRS "GaTypes.h"
           PATH_SUFFIXES "gangsta" )

FIND_LIBRARY( GANGSTA_LIBRARIES
              NAMES "gangsta"
              PATH_SUFFIXES "gangsta" )

# handle the QUIETLY and REQUIRED arguments and set GANGSTA_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "Gangsta" DEFAULT_MSG GANGSTA_INCLUDE_DIRS GANGSTA_LIBRARIES )

MARK_AS_ADVANCED( GANGSTA_INCLUDE_DIRS GANGSTA_LIBRARIES )
