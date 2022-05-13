#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
# Updates:  Allan
#

###############
# CMake setup #
###############
# We need a target-specific property INCLUDE_DIRECTORIES which
# is part of CMake 2.8.8 and later.
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.8 FATAL_ERROR )

PROJECT( "evemu" )
SET( PROJECT_DESCRIPTION_SUMMARY "A server emulator for EVE Online" )
SET( PROJECT_VENDOR              "The EVEmu development team" )
SET( PROJECT_VERSION             "this is not used anymore, CMake pulls version from Git Tree Info (cmake/GitTreeInfo.cmake)" )
SET( PROJECT_REPOSITORY          "https://github.com/EvEmu-Project/evemu_Crucible" )

# Append our module path
LIST( APPEND CMAKE_MODULE_PATH
      "${PROJECT_SOURCE_DIR}/cmake" )
# Enable testing with CTest
ENABLE_TESTING()

###########
# Options #
###########
SET( AUTO_DOWNLOAD_DEPENDENCIES OFF
     CACHE BOOL "When CHECKED, CMake will automatically download Boost and MySQL, otherwise, CMake will look for them in locations specified by 'Boost_DIR', 'Boost_INCLUDE_DIR', 'MYSQL_INCLUDE_DIRS', and 'MYSQL_LIBRARIES'" )
SET( EVEMU_ROOT ".."
     CACHE PATH "The root directory of EVEmu workspace." )
SET( TIXML_USE_STL ON
     CACHE BOOL "tinyxml will use native STL." )

IF( CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_IMPORT ""
       CACHE FILEPATH "Path to an import script generated during a native build." )
ELSE( CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_EXPORT ""
       CACHE FILEPATH "Path to an import script to generate (for cross-compiling)." )
ENDIF( CMAKE_CROSSCOMPILING )

MARK_AS_ADVANCED(
  AUTO_DOWNLOAD_DEPENDENCIES
  EVEMU_ROOT
  EVEMU_TARGETS_IMPORT
  EVEMU_TARGETS_EXPORT
  TIXML_USE_STL
  )

#################
# Configuration #
#################
INCLUDE( "DetectScm" )

#
# Setup some recommended compiler flags.
#
# Intentionally specify the CMAKE_CXX_FLAGS as last to allow user
# override the setup.
#
# Switch to the C++11 standard (required)
SET( CMAKE_CXX_FLAGS "-std=gnu++11 -march=native -Werror=return-type -fabi-version=14 ${CMAKE_CXX_FLAGS}" )
# -Wall -Wextra -Weffc++ -Wconversion -Wdouble-promotion
# -pg -ggdb -pedantic
# -fsanitize=thread  -fsanitize=address

#
# Try to determine the exact source version using SCM.
#
# Also check that the generated version string starts
# with PROJECT_VERSION, issue a warning otherwise.
#
DETECT_SCM( "${PROJECT_SOURCE_DIR}" SCM_VERSION SCM_REPOSITORY )
IF( SCM_VERSION AND SCM_REPOSITORY )
  # Check that SCM_VERSION starts with PROJECT_VERSION
  STRING( FIND "${SCM_VERSION}" "${PROJECT_VERSION}" VERSION_INDEX )

  # Set project version and repository
  SET( PROJECT_VERSION    "${SCM_VERSION}" )
  SET( PROJECT_REPOSITORY "${SCM_REPOSITORY}" )

  ###########################
  # Commit / Branch Strings #
  ###########################

  # Get the current working branch
  execute_process(
      COMMAND git rev-parse --abbrev-ref HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Get the latest commit hash
  execute_process(
      COMMAND git rev-parse HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  ADD_DEFINITIONS("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
  ADD_DEFINITIONS("-DGIT_BRANCH=\"${GIT_BRANCH}\"")


ENDIF( SCM_VERSION AND SCM_REPOSITORY )

#########
# Tests #
#########
INCLUDE( "CheckCXXSourceCompiles" )
INCLUDE( "CheckCXXSymbolExists" )
INCLUDE( "CheckIncludeFileCXX" )

# Headers
CHECK_INCLUDE_FILE_CXX( "crtdbg.h"   HAVE_CRTDBG_H )
CHECK_INCLUDE_FILE_CXX( "inttypes.h" HAVE_INTTYPES_H )
CHECK_INCLUDE_FILE_CXX( "sys/stat.h" HAVE_SYS_STAT_H )
CHECK_INCLUDE_FILE_CXX( "sys/time.h" HAVE_SYS_TIME_H )
CHECK_INCLUDE_FILE_CXX( "vld.h"      HAVE_VLD_H )

# Keywords
CHECK_CXX_SOURCE_COMPILES(
  "int main() { __asm int 3 }\n"
  HAVE___ASM )

# cfloat, cmath
CHECK_CXX_SYMBOL_EXISTS( asinh    "cmath"  HAVE_ASINH )
CHECK_CXX_SYMBOL_EXISTS( finite   "cmath"  HAVE_FINITE )
CHECK_CXX_SYMBOL_EXISTS( isfinite "cmath"  HAVE_ISFINITE )
CHECK_CXX_SYMBOL_EXISTS( isnan    "cmath"  HAVE_ISNAN )
CHECK_CXX_SYMBOL_EXISTS( _finite  "cfloat" HAVE__FINITE )
CHECK_CXX_SYMBOL_EXISTS( _isnan   "cfloat" HAVE__ISNAN )
#CHECK_CXX_SYMBOL_EXISTS( __finite "cmath"  HAVE___FINITE )
#CHECK_CXX_SYMBOL_EXISTS( __isnan  "cmath"  HAVE___ISNAN )

CHECK_CXX_SOURCE_COMPILES(
  "#include <cmath>\nint main() { return std::finite( 0 ); }\n"
  HAVE_STD_FINITE )
CHECK_CXX_SOURCE_COMPILES(
  "#include <cmath>\nint main() { return std::isfinite( 0 ); }\n"
  HAVE_STD_ISFINITE )
CHECK_CXX_SOURCE_COMPILES(
  "#include <cmath>\nint main() { return std::isnan( 0 ); }\n"
  HAVE_STD_ISNAN )

# cstdarg
CHECK_CXX_SYMBOL_EXISTS( va_copy "cstdarg" HAVE_VA_COPY )

# cstdio
CHECK_CXX_SYMBOL_EXISTS(  snprintf "cstdio" HAVE_SNPRINTF )
CHECK_CXX_SYMBOL_EXISTS( vsnprintf "cstdio" HAVE_VSNPRINTF )
CHECK_CXX_SYMBOL_EXISTS(  asprintf "cstdio" HAVE_ASPRINTF )
CHECK_CXX_SYMBOL_EXISTS( vasprintf "cstdio" HAVE_VASPRINTF )

# cstdlib
CHECK_CXX_SYMBOL_EXISTS( strtoll  "cstdlib" HAVE_STRTOLL )
CHECK_CXX_SYMBOL_EXISTS( strtoull "cstdlib" HAVE_STRTOULL )
CHECK_CXX_SYMBOL_EXISTS( strtof   "cstdlib" HAVE_STRTOF )

# cstring
CHECK_CXX_SYMBOL_EXISTS( strdup      "cstring" HAVE_STRDUP )
CHECK_CXX_SYMBOL_EXISTS( strcasecmp  "cstring" HAVE_STRCASECMP )
CHECK_CXX_SYMBOL_EXISTS( strncasecmp "cstring" HAVE_STRNCASECMP )

# ctime
CHECK_CXX_SYMBOL_EXISTS( localtime_r "ctime" HAVE_LOCALTIME_R )
CHECK_CXX_SYMBOL_EXISTS( localtime_s "ctime" HAVE_LOCALTIME_S )

############
# Packages #
############
INCLUDE( "BuildPackage" )


BUILD_PACKAGE( "Boost" )
BUILD_PACKAGE( "MySQL" )
BUILD_PACKAGE( "TinyXML" )
BUILD_PACKAGE( "UTF8CPP" )
BUILD_PACKAGE( "ZLIB" )

FIND_PACKAGE( "Threads" REQUIRED )

ADD_SUBDIRECTORY( "dep/gangsta" )
ADD_SUBDIRECTORY( "dep/utils" )

SET( CMAKE_NET_LIBS
  "${CMAKE_NET_LIBS_INIT}" )
SET( CMAKE_THREAD_LIBS
  "${CMAKE_THREAD_LIBS_INIT}" )

MARK_AS_ADVANCED(
  Boost_DIR
  Boost_LIB_DIAGNOSTIC_DEFINITIONS
  )

#################
# EVEmu targets #
#################
INCLUDE( "TargetBuildPCH" )
INCLUDE( "TargetIncludeDirectories" )

# Advertise config.h
ADD_DEFINITIONS( "-DHAVE_CONFIG_H")
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}" )
# Generate config.h
CONFIGURE_FILE(
  "config.h.in"
  "config.h"
  ESCAPE_QUOTES @ONLY
  )

# Import targets if any
IF( EVEMU_TARGETS_IMPORT )
  INCLUDE( "${EVEMU_TARGETS_IMPORT}" )
ENDIF( EVEMU_TARGETS_IMPORT )

# Add all targets
ADD_SUBDIRECTORY( "src/eve-core" )
ADD_SUBDIRECTORY( "src/eve-xmlpktgen" )
ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )
