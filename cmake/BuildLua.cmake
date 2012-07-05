# - Build config of Lua
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "Lua" )
SET( PKG_FOUND "${LUA_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( LUA_URL
  "http://www.lua.org/ftp/lua-5.2.1.tar.gz"
  CACHE STRING "URL of the Lua source archive" )
MARK_AS_ADVANCED( LUA_URL )

# Setup the needed variables
SET( PKG_URL "${LUA_URL}" )
SET( PKG_MD5 "ae08f641b45d737d12d30291a5e5f6e3" )
SET( PKG_ARCHIVE_PREFIX "/lua-5.2.1" )

SET( PKG_CMAKELISTS "
INCLUDE_DIRECTORIES(
  \"\${CMAKE_CURRENT_SOURCE_DIR}/src\"
  )
ADD_LIBRARY(
  \"lua\"
  # Source:
  \"src/lapi.h\"
  \"src/lapi.c\"
  \"src/lauxlib.h\"
  \"src/lauxlib.c\"
  \"src/lbaselib.c\"
  \"src/lbitlib.c\"
  \"src/lcode.h\"
  \"src/lcode.c\"
  \"src/lcorolib.c\"
  \"src/lctype.h\"
  \"src/lctype.c\"
  \"src/ldblib.c\"
  \"src/ldebug.h\"
  \"src/ldebug.c\"
  \"src/ldo.h\"
  \"src/ldo.c\"
  \"src/ldump.c\"
  \"src/lfunc.h\"
  \"src/lfunc.c\"
  \"src/lgc.h\"
  \"src/lgc.c\"
  \"src/linit.c\"
  \"src/liolib.c\"
  \"src/llex.h\"
  \"src/llex.c\"
  \"src/llimits.h\"
  \"src/lmathlib.c\"
  \"src/lmem.h\"
  \"src/lmem.c\"
  \"src/loadlib.c\"
  \"src/lobject.h\"
  \"src/lobject.c\"
  \"src/lopcodes.h\"
  \"src/lopcodes.c\"
  \"src/loslib.c\"
  \"src/lparser.h\"
  \"src/lparser.c\"
  \"src/lstate.h\"
  \"src/lstate.c\"
  \"src/lstring.h\"
  \"src/lstring.c\"
  \"src/lstrlib.c\"
  \"src/ltable.h\"
  \"src/ltable.c\"
  \"src/ltablib.c\"
  \"src/ltm.h\"
  \"src/ltm.c\"
  \"src/lua.h\"
  \"src/lua.hpp\"
# Skip Lua interpreter source
# \"src/lua.c\"
# Skip Lua compiler source
# \"src/luac.c\"
  \"src/luaconf.h\"
  \"src/lualib.h\"
  \"src/lundump.h\"
  \"src/lundump.c\"
  \"src/lvm.h\"
  \"src/lvm.c\"
  \"src/lzio.h\"
  \"src/lzio.c\"
  )

SET( LUA_INCLUDE_DIR
  \"\${CMAKE_CURRENT_SOURCE_DIR}/src\"
  PARENT_SCOPE )
SET( LUA_LIBRARIES
  \"lua\"
  PARENT_SCOPE )
" )
