# - Build config of Argon2
#
# Argon2 is acquired from a fixed upstream source archive and verified by
# SHA-256 before it is extracted. The project builds the reference library
# without its optional worker threads because authentication uses p=1.

SET( PKG_FOUND FALSE )
SET( ARGON2_URL
  "https://github.com/P-H-C/phc-winner-argon2/archive/refs/tags/20190702.tar.gz"
  CACHE STRING "URL of the Argon2 source archive" )
SET( ARGON2_SHA256
  "DAF972A89577F8772602BF2EB38B6A3DD3D922BF5724D45E7F9589B5E830442C"
  CACHE STRING "SHA-256 of the Argon2 source archive" )
MARK_AS_ADVANCED( ARGON2_URL ARGON2_SHA256 )

SET( PKG_URL "${ARGON2_URL}" )
SET( PKG_SHA256 "${ARGON2_SHA256}" )
SET( PKG_ARCHIVE "argon2-20190702.tar.gz" )
SET( PKG_ARCHIVE_PREFIX "/phc-winner-argon2-20190702" )

SET( PKG_CMAKELISTS "
ADD_LIBRARY(
  argon2 STATIC
  src/argon2.c
  src/core.c
  src/encoding.c
  src/ref.c
  src/blake2/blake2b.c
  )
TARGET_COMPILE_DEFINITIONS( argon2 PRIVATE ARGON2_NO_THREADS )
TARGET_INCLUDE_DIRECTORIES(
  argon2 PUBLIC
  \"\${CMAKE_CURRENT_SOURCE_DIR}/include\"
  \"\${CMAKE_CURRENT_SOURCE_DIR}/src\"
  )
SET( ARGON2_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}/include\"
  PARENT_SCOPE )
SET( ARGON2_LIBRARIES argon2 PARENT_SCOPE )
" )
