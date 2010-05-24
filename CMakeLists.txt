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

SET( CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake"
     "${CMAKE_MODULE_PATH}" )

INCLUDE( "CheckSymbolExists" )

INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestBigEndian" )
INCLUDE( "TestX64" )

# Options
OPTION( EVE_COLLECTOR
        "Build eve-collector."
        OFF )
OPTION( EVE_TOOL
        "Build eve-tool."
        OFF )
OPTION( EVE_EXTRACT_SVN_REV
        "Extract current revision using SVN tools."
        OFF )
OPTION( TINYXML_USE_STL
        "tinyxml will use native STL."
        ON )

SET( PROJECT_VERSION "0.7-trunk" )

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

# Check for dependencies
FIND_PACKAGE( "MySql" 5.0 REQUIRED )
FIND_PACKAGE( "ZLIB" REQUIRED )

IF( EVE_EXTRACT_SVN_REV )
    FIND_PACKAGE( "Subversion" REQUIRED )
ENDIF( EVE_EXTRACT_SVN_REV )

FIND_PACKAGE( "Doxygen" )

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
IF( EVE_EXTRACT_SVN_REV )
    Subversion_WC_INFO( "${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}" )

    SET( PROJECT_VERSION "${PROJECT_VERSION}-r${${PROJECT_NAME}_WC_LAST_CHANGED_REV}" )
ENDIF( EVE_EXTRACT_SVN_REV )

# Setup config.h
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/config.h.cmake"
                "${PROJECT_BINARY_DIR}/config.h"
                ESCAPE_QUOTES @ONLY )

ADD_DEFINITIONS( "-DHAVE_CONFIG_H" )
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}" )

#############################
# Recurse to subdirectories #
#############################
ADD_SUBDIRECTORY( "dep" )
ADD_SUBDIRECTORY( "src/common" )
ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )
ADD_SUBDIRECTORY( "src/xmlpktgen" )

IF( EVE_COLLECTOR )
    ADD_SUBDIRECTORY( "src/eve-collector" )
ENDIF( EVE_COLLECTOR )

IF( EVE_TOOL )
    ADD_SUBDIRECTORY( "src/eve-tool" )
ENDIF( EVE_TOOL )

IF( DOXYGEN_FOUND )
    ADD_SUBDIRECTORY( "doc" )
ENDIF( DOXYGEN_FOUND )
