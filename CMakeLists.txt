#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

###############
# CMake setup #
###############
# We need the CheckCXXSymbolExists module which is distributed
# with CMake 2.8.6 and later.
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.6 FATAL_ERROR )

PROJECT( "evemu" )
SET( PROJECT_DESCRIPTION_SUMMARY "A server emulator for EVE Online" )
SET( PROJECT_VENDOR              "The EVEmu development team" )
SET( PROJECT_VERSION             "0.7.11" )
SET( PROJECT_REPOSITORY          "https://github.com/evemuproject/evemu_server" )

LIST( APPEND CMAKE_MODULE_PATH
      "${PROJECT_SOURCE_DIR}/cmake" )

###########
# Options #
###########
SET( EVEMU_ROOT ".."
     CACHE PATH "The root directory of EVEmu workspace." )
MARK_AS_ADVANCED( EVEMU_ROOT )

IF( CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_IMPORT ""
       CACHE FILEPATH "Path to an import script generated during a native build." )
  MARK_AS_ADVANCED( EVEMU_TARGETS_IMPORT )
ELSE( CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_EXPORT ""
       CACHE FILEPATH "Path to an import script to generate (for cross-compiling)." )
  MARK_AS_ADVANCED( EVEMU_TARGETS_EXPORT )
ENDIF( CMAKE_CROSSCOMPILING )

#########
# Tests #
#########
INCLUDE( "CheckCXXSymbolExists" )
INCLUDE( "CheckIncludeFile" )
INCLUDE( "DetectScm" )
INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestX64" )

TEST_X64( X64 )

# Headers
CHECK_INCLUDE_FILE( "crtdbg.h"   HAVE_CRTDBG_H )
CHECK_INCLUDE_FILE( "inttypes.h" HAVE_INTTYPES_H )
CHECK_INCLUDE_FILE( "vld.h"      HAVE_VLD_H )

# cfloat, cmath
CHECK_CXX_SYMBOL_EXISTS( asinh         "cmath"  HAVE_ASINH )
CHECK_CXX_SYMBOL_EXISTS( _finite       "cfloat" HAVE__FINITE )
CHECK_CXX_SYMBOL_EXISTS( _isnan        "cfloat" HAVE__ISNAN )
CHECK_CXX_SYMBOL_EXISTS( __finite      "cmath"  HAVE___FINITE )
CHECK_CXX_SYMBOL_EXISTS( __isnan       "cmath"  HAVE___ISNAN )
CHECK_CXX_SYMBOL_EXISTS( std::finite   "cmath"  HAVE_STD_FINITE )
CHECK_CXX_SYMBOL_EXISTS( std::isfinite "cmath"  HAVE_STD_ISFINITE )
CHECK_CXX_SYMBOL_EXISTS( std::isnan    "cmath"  HAVE_STD_ISNAN )

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

# cstring
CHECK_CXX_SYMBOL_EXISTS( strdup      "cstring" HAVE_STRDUP )
CHECK_CXX_SYMBOL_EXISTS( strcasecmp  "cstring" HAVE_STRCASECMP )
CHECK_CXX_SYMBOL_EXISTS( strncasecmp "cstring" HAVE_STRNCASECMP )

# ctime
CHECK_CXX_SYMBOL_EXISTS( localtime_r "ctime" HAVE_LOCALTIME_R )
CHECK_CXX_SYMBOL_EXISTS( localtime_s "ctime" HAVE_LOCALTIME_S )

# sys/socket.h
CHECK_CXX_SYMBOL_EXISTS( MSG_NOSIGNAL "sys/socket.h" HAVE_MSG_NOSIGNAL )
CHECK_CXX_SYMBOL_EXISTS( SO_NOSIGPIPE "sys/socket.h" HAVE_SO_NOSIGPIPE )

#################
# Configuration #
#################
IF( MSVC )
  #
  # This is here as we usually use pre-built MySQL binaries which are
  # compiled with /MT (use static libc); building with /MD (use DLL libc)
  # results in linker warning during the linking stage, however the resultant
  # binary works and is even a little bit smaller due to the dynamic linking.
  #
  # For now we supress the linker warning by forcing static libc.
  #
  #STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS}" )
  #STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}" )
  #STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
  #STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
  #STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
ELSEIF( CMAKE_COMPILER_IS_GNUCXX )
  #
  # Setup some recommended compiler flags.
  #
  # Intentionally specify the CMAKE_CXX_FLAGS as last to allow user
  # override the setup.
  #
  SET( CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}" )
ENDIF( MSVC )

# Pull version from SCM, if any
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

#################
# EVEmu targets #
#################
# Import targets if any
IF( EVEMU_TARGETS_IMPORT )
  INCLUDE( "${EVEMU_TARGETS_IMPORT}" )
ENDIF( EVEMU_TARGETS_IMPORT )

# Advertise config.h
ADD_DEFINITIONS( "-DHAVE_CONFIG_H" )
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}" )

# Add all targets
ADD_SUBDIRECTORY( "doc" )
ADD_SUBDIRECTORY( "src/eve-core" )
ADD_SUBDIRECTORY( "src/eve-xmlpktgen" )
ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )
ADD_SUBDIRECTORY( "src/eve-collector" )
ADD_SUBDIRECTORY( "src/eve-tool" )

# Generate config.h
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/config.h.in"
                "${PROJECT_BINARY_DIR}/config.h"
                ESCAPE_QUOTES @ONLY )

###############
# CPack setup #
###############
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION_SUMMARY}" )
SET( CPACK_PACKAGE_VENDOR              "${PROJECT_VENDOR}" )
SET( CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}" )

INCLUDE( "CPack" )
