#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
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
SET( PROJECT_VERSION             "0.7.11" )
SET( PROJECT_REPOSITORY          "https://github.com/evemuproject/evemu_server" )

# Append our module path
LIST( APPEND CMAKE_MODULE_PATH
      "${PROJECT_SOURCE_DIR}/cmake" )
# Enable testing with CTest
ENABLE_TESTING()

###########
# Options #
###########
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
# Setup some default libraries.
#
# We need a bunch of additional network libraries when building
# for Windows.
#
IF( WIN32 )
  # Create a list of the additional libraries
  SET( CMAKE_NET_LIBS_INIT "ws2_32" "mswsock" )
ENDIF( WIN32 )

#
# Setup some recommended compiler flags.
#
# Intentionally specify the CMAKE_CXX_FLAGS as last to allow user
# override the setup.
#
IF( MSVC )
  # Turn off some stupid whiny warnings
  ADD_DEFINITIONS(
    "-D_CRT_SECURE_NO_WARNINGS"
    "-D_CRT_SECURE_NO_DEPRECATE"
    "-D_CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES"
    "-D_SCL_SECURE_NO_WARNINGS"
    )
  # Disable warnings:
  #   C4244: '=' : conversion from 'double' to 'int', possible loss of data
  #   C4355: 'this' used in base member initializer list
  SET( CMAKE_CXX_FLAGS "/wd4244 /wd4355 ${CMAKE_CXX_FLAGS}" )
ELSEIF( CMAKE_COMPILER_IS_GNUCXX )
  # Switch to the C++0x standard (required)
  SET( CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}" )
ENDIF( MSVC )

#
# Try to detemine the exact source version using SCM.
#
# Also check that the generated version string starts
# with PROJECT_VERSION, issue a warning otherwise.
#
DETECT_SCM( "${PROJECT_SOURCE_DIR}" SCM_VERSION SCM_REPOSITORY )
IF( SCM_VERSION AND SCM_REPOSITORY )
  # Check that SCM_VERSION starts with PROJECT_VERSION
  STRING( FIND "${SCM_VERSION}" "${PROJECT_VERSION}" VERSION_INDEX )
  IF( NOT VERSION_INDEX EQUAL 0 )
    MESSAGE( AUTHOR_WARNING "Version mismatch, please update PROJECT_VERSION in CMakeLists.txt" )
  ENDIF( NOT VERSION_INDEX EQUAL 0 )

  # Set project version and repository
  SET( PROJECT_VERSION    "${SCM_VERSION}" )
  SET( PROJECT_REPOSITORY "${SCM_REPOSITORY}" )
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
CHECK_INCLUDE_FILE_CXX( "tr1/tuple"  HAVE_TR1_PREFIX )
CHECK_INCLUDE_FILE_CXX( "vld.h"      HAVE_VLD_H )
CHECK_INCLUDE_FILE_CXX( "windows.h"  HAVE_WINDOWS_H )
CHECK_INCLUDE_FILE_CXX( "winsock2.h" HAVE_WINSOCK2_H )

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
CHECK_CXX_SYMBOL_EXISTS( __finite "cmath"  HAVE___FINITE )
CHECK_CXX_SYMBOL_EXISTS( __isnan  "cmath"  HAVE___ISNAN )

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

FIND_PACKAGE( "Doxygen" )
FIND_PACKAGE( "Threads" REQUIRED )

BUILD_PACKAGE( "Boost" )
BUILD_PACKAGE( "Lua51" )
BUILD_PACKAGE( "MySQL" )
BUILD_PACKAGE( "TinyXML" )
BUILD_PACKAGE( "UTF8CPP" )
BUILD_PACKAGE( "ZLIB" )

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
ADD_DEFINITIONS( "-DHAVE_CONFIG_H" )
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
ADD_SUBDIRECTORY( "src/eve-collector" )
ADD_SUBDIRECTORY( "src/eve-tool" )
ADD_SUBDIRECTORY( "src/eve-test" )

IF( DOXYGEN_FOUND )
  ADD_SUBDIRECTORY( "doc" )
ELSE()
  MESSAGE( WARNING "If you require generated documentation, please install Doxygen" )
ENDIF( DOXYGEN_FOUND )

###############
# CPack setup #
###############
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION_SUMMARY}" )
SET( CPACK_PACKAGE_VENDOR              "${PROJECT_VENDOR}" )
SET( CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}" )

INCLUDE( "CPack" )
