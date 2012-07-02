#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

FIND_PACKAGE( "Lua" QUIET )
MARK_AS_ADVANCED( Lua_DIR )
IF( LUA_FOUND )
  # Lua found in system
  RETURN()
ENDIF()

# Disable safety warnings because this is a C library
IF( MSVC )
  ADD_DEFINITIONS( "-D_CRT_SECURE_NO_WARNINGS" )
ENDIF( MSVC )

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
ADD_LIBRARY(
  "lua"
  # Source:
  "src/lapi.h"
  "src/lapi.c"
  "src/lauxlib.h"
  "src/lauxlib.c"
  "src/lbaselib.c"
  "src/lcode.h"
  "src/lcode.c"
  "src/ldblib.c"
  "src/ldebug.h"
  "src/ldebug.c"
  "src/ldo.h"
  "src/ldo.c"
  "src/ldump.c"
  "src/lfunc.h"
  "src/lfunc.c"
  "src/lgc.h"
  "src/lgc.c"
  "src/linit.c"
  "src/liolib.c"
  "src/llex.h"
  "src/llex.c"
  "src/llimits.h"
  "src/lmathlib.c"
  "src/lmem.h"
  "src/lmem.c"
  "src/loadlib.c"
  "src/lobject.h"
  "src/lobject.c"
  "src/lopcodes.h"
  "src/lopcodes.c"
  "src/loslib.c"
  "src/lparser.h"
  "src/lparser.c"
  "src/lstate.h"
  "src/lstate.c"
  "src/lstring.h"
  "src/lstring.c"
  "src/lstrlib.c"
  "src/ltable.h"
  "src/ltable.c"
  "src/ltablib.c"
  "src/ltm.h"
  "src/ltm.c"
  "src/lua.h"
  "src/lua.c"
  "src/luac.c"
  "src/luaconf.h"
  "src/lualib.h"
  "src/lundump.h"
  "src/lundump.c"
  "src/lvm.h"
  "src/lvm.c"
  "src/lzio.h"
  "src/lzio.c"
  "src/print.c"
  )

SET( LUA_INCLUDE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  PARENT_SCOPE )
SET( LUA_LIBRARIES
  "lua"
  PARENT_SCOPE )
