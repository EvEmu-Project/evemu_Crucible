# - Build config of zlib
# Defines several variables needed for automatic
# download and build of source archive.
#

# Version 1.2.3 (July 2005) eliminates potential security
# vulnerabilities in zlib 1.2.1 and 1.2.2, so all users of those
# versions should upgrade immediately.
FIND_PACKAGE( "ZLIB" 1.2.3 )
SET( PKG_FOUND "${ZLIB_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( ZLIB_URL
  "http://downloads.sourceforge.net/project/libpng/zlib/1.2.7/zlib-1.2.7.tar.bz2"
  CACHE STRING "URL of the zlib source archive" )
MARK_AS_ADVANCED( ZLIB_URL )

# Setup the needed variables
SET( PKG_URL "${ZLIB_URL}" )
SET( PKG_MD5 "2ab442d169156f34c379c968f3f482dd" )
SET( PKG_ARCHIVE_PREFIX "/zlib-1.2.7" )

SET( PKG_CMAKELISTS "
INCLUDE_DIRECTORIES(
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  )
ADD_LIBRARY(
  \"zlib\"
  # Source:
  \"adler32.c\"
  \"compress.c\"
  \"crc32.h\"
  \"crc32.c\"
  \"deflate.h\"
  \"deflate.c\"
  \"gzclose.c\"
  \"gzguts.h\"
  \"gzlib.c\"
  \"gzread.c\"
  \"gzwrite.c\"
  \"infback.c\"
  \"inffast.h\"
  \"inffast.c\"
  \"inffixed.h\"
  \"inflate.h\"
  \"inflate.c\"
  \"inftrees.h\"
  \"inftrees.c\"
  \"trees.h\"
  \"trees.c\"
  \"uncompr.c\"
  \"zconf.h\"
  \"zlib.h\"
  \"zutil.h\"
  \"zutil.c\"
  )

SET( ZLIB_INCLUDE_DIR
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  PARENT_SCOPE )
SET( ZLIB_LIBRARY
  \"zlib\"
  PARENT_SCOPE )
" )
