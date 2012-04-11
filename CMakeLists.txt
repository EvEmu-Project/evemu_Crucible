#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

###############
# CMake setup #
###############
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 FATAL_ERROR )

PROJECT( "evemu" )
SET( PROJECT_DESCRIPTION_SUMMARY "A server emulator for EVE Online" )
SET( PROJECT_VENDOR              "The EVEmu development team" )

SET( CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake"
     "${CMAKE_MODULE_PATH}" )

###########
# Options #
###########
SET( EVEMU_ROOT_DIR "..////"
     CACHE PATH "The root of EVEmu workspace." )
MARK_AS_ADVANCED( EVEMU_ROOT_DIR )

#########
# Tests #
#########
INCLUDE( "CheckSymbolExists" )
INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestX64" )

TEST_X64( X64 )
CHECK_SYMBOL_EXISTS( "__FreeBSD__" "" FREE_BSD )
SET( GNUC ${CMAKE_COMPILER_IS_GNUCXX} )

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
ELSEIF( GNUC )
  #
  # Setup some recommended compiler flags.
  #
  # Intentionally specify the CMAKE_CXX_FLAGS as last to allow user
  # override the setup.
  #
  SET( CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}" )
ENDIF( MSVC )

SET( EVEMU_SOURCE_SCM ""
     CACHE STRING "Source SCM in use; valid values: '', 'svn', 'git'" )

IF( EVEMU_SOURCE_SCM STREQUAL "svn" )
  FIND_PACKAGE( "Subversion" REQUIRED )
  Subversion_WC_INFO( "${PROJECT_SOURCE_DIR}" EVEMU_SVN )
  SET( PROJECT_VERSION "r${EVEMU_SVN_WC_LAST_CHANGED_REV}" )
ELSEIF( EVEMU_SOURCE_SCM STREQUAL "git" )
  INCLUDE( "GitTreeInfo" )
  GIT_TREE_INFO( "${PROJECT_SOURCE_DIR}" EVEMU_GIT )
  SET( PROJECT_VERSION "${EVEMU_GIT_VERSION}" )
ENDIF( EVEMU_SOURCE_SCM STREQUAL "svn" )

######################
# Advertise config.h #
######################
ADD_DEFINITIONS( "-DHAVE_CONFIG_H" )
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}" )

##################
# Targets import #
##################
IF( CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_IMPORT ""
       CACHE FILEPATH "Path to an import script generated during a native build." )
  MARK_AS_ADVANCED( EVEMU_TARGETS_IMPORT )

  IF( EVEMU_TARGETS_IMPORT )
    INCLUDE( "${EVEMU_TARGETS_IMPORT}" )
  ENDIF( EVEMU_TARGETS_IMPORT )
ENDIF( CMAKE_CROSSCOMPILING )

#################
# EVEmu targets #
#################
SET( EVEMU_COLLECTOR_ENABLE OFF
     CACHE BOOL "Build eve-collector." )
SET( EVEMU_TOOL_ENABLE      OFF
     CACHE BOOL "Build eve-tool." )

ADD_SUBDIRECTORY( "doc" )
ADD_SUBDIRECTORY( "src/common" )

IF( NOT CMAKE_CROSSCOMPILING )
  ADD_SUBDIRECTORY( "src/xmlpktgen" )
ENDIF( NOT CMAKE_CROSSCOMPILING )

ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )

IF( EVEMU_COLLECTOR_ENABLE )
  ADD_SUBDIRECTORY( "src/eve-collector" )
ENDIF( EVEMU_COLLECTOR_ENABLE )

IF( EVEMU_TOOL_ENABLE )
  ADD_SUBDIRECTORY( "src/eve-tool" )
ENDIF( EVEMU_TOOL_ENABLE )

##################
# Targets export #
##################
IF( NOT CMAKE_CROSSCOMPILING )
  SET( EVEMU_TARGETS_EXPORT ""
       CACHE FILEPATH "Path to an import script to generate (for cross-compiling)." )
  MARK_AS_ADVANCED( EVEMU_TARGETS_EXPORT )

  IF( EVEMU_TARGETS_EXPORT )
    EXPORT( TARGETS "xmlpktgen"
            FILE "${EVEMU_TARGETS_EXPORT}" )
  ENDIF( EVEMU_TARGETS_EXPORT )
ENDIF( NOT CMAKE_CROSSCOMPILING )

#####################
# Generate config.h #
#####################
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
