#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

#################
# Initial setup #
#################
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 FATAL_ERROR )

PROJECT( "evemu" )
SET( PROJECT_VERSION "0.7-experimental" )

SET( CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake"
     "${CMAKE_MODULE_PATH}" )

INCLUDE( "CheckSymbolExists" )

INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestBigEndian" )
INCLUDE( "TestX64" )

# Options
SET( EVEMU_COLLECTOR_ENABLE          OFF
     CACHE BOOL "Enable eve-collector." )
SET( EVEMU_EXTRACT_SVN_REV           OFF
     CACHE BOOL "Extract current revision using SVN tools." )
SET( EVEMU_ROOT_DIR                  ".."
     CACHE PATH "The root of EVEmu workspace." )
SET( EVEMU_SERVER_SHOW_LOGIN_MESSAGE ON
     CACHE BOOL "Show a message to a client when logging in." )
SET( EVEMU_TOOL_ENABLE               OFF
     CACHE BOOL "Enable eve-tool." )

SET( TINYXML_USE_STL                 ON
     CACHE BOOL "tinyxml will use native STL." )

#########
# Tests #
#########
# Examine hardware
TEST_X64( X64 )
TEST_BIG_ENDIAN( BIG_ENDIAN )

# Examine OS
CHECK_SYMBOL_EXISTS( "__FreeBSD__" "" FREE_BSD )

# Examine compiler
SET( GNUC ${CMAKE_COMPILER_IS_GNUCXX} )

# Check for necessary dependencies
FIND_PACKAGE( "MySQL" 5.0 REQUIRED )
FIND_PACKAGE( "Threads" REQUIRED )
FIND_PACKAGE( "ZLIB" REQUIRED )

IF( EVEMU_EXTRACT_SVN_REV )
    FIND_PACKAGE( "Subversion" REQUIRED )
ENDIF( EVEMU_EXTRACT_SVN_REV )

# Check for other dependencies
FIND_PACKAGE( "Doxygen" QUIET )
FIND_PACKAGE( "Gangsta" QUIET )
FIND_PACKAGE( "TinyXML" QUIET )
FIND_PACKAGE( "UTF8CPP" QUIET )

#################
# Configuration #
#################
# Platform-specific configuration
IF( WIN32 )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

    SET( CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} wsock32.lib" )
ENDIF( WIN32 )

# Update PROJECT_VERSION appropriately
IF( EVEMU_EXTRACT_SVN_REV )
    Subversion_WC_INFO( "${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}" )

    SET( PROJECT_VERSION "${PROJECT_VERSION}-r${${PROJECT_NAME}_WC_LAST_CHANGED_REV}" )
ENDIF( EVEMU_EXTRACT_SVN_REV )

# Setup config.h
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/config.h.cmake"
                "${PROJECT_BINARY_DIR}/config.h"
                ESCAPE_QUOTES @ONLY )

ADD_DEFINITIONS( "-DHAVE_CONFIG_H" )
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}" )

############################
# Distributed dependencies #
############################
ADD_SUBDIRECTORY( "dep/utils" )

IF( NOT GANGSTA_FOUND )
    ADD_SUBDIRECTORY( "dep/gangsta" )

    SET( GANGSTA_FOUND       TRUE )
    SET( GANGSTA_INCLUDE_DIR "${gangsta_INCLUDE_DIR}" )
    SET( GANGSTA_LIBRARIES   "gangsta" )
ENDIF( NOT GANGSTA_FOUND )

IF( NOT TINYXML_FOUND )
    ADD_SUBDIRECTORY( "dep/tinyxml" )

    SET( TINYXML_FOUND       TRUE )
    SET( TINYXML_INCLUDE_DIR "${tinyxml_INCLUDE_DIR}" )
    SET( TINYXML_LIBRARIES   "tinyxml" )
ENDIF( NOT TINYXML_FOUND )

IF( NOT UTF8CPP_FOUND )
    ADD_SUBDIRECTORY( "dep/utf8cpp" )

    SET( UTF8CPP_FOUND       TRUE )
    SET( UTF8CPP_INCLUDE_DIR "${utf8cpp_INCLUDE_DIR}" )
    SET( UTF8CPP_LIBRARIES   "utf8cpp" )
ENDIF( NOT UTF8CPP_FOUND )

###########################
# Generated documentation #
###########################
IF( DOXYGEN_FOUND )
    ADD_SUBDIRECTORY( "doc" )
ELSE( DOXYGEN_FOUND )
    MESSAGE( WARNING "Doxygen not found: a generated documentation will not be available." )
ENDIF( DOXYGEN_FOUND )

################
# EVEmu source #
################
ADD_SUBDIRECTORY( "src/common" )
ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )
ADD_SUBDIRECTORY( "src/xmlpktgen" )

IF( EVEMU_COLLECTOR_ENABLE )
    ADD_SUBDIRECTORY( "src/eve-collector" )
ENDIF( EVEMU_COLLECTOR_ENABLE )

IF( EVEMU_TOOL_ENABLE )
    ADD_SUBDIRECTORY( "src/eve-tool" )
ENDIF( EVEMU_TOOL_ENABLE )
