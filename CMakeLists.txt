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

INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestBigEndian" )
INCLUDE( "TestX64" )

# Custom options
OPTION( EVE_COLLECTOR
        "Include eve-collector."
        OFF )
OPTION( EVE_TOOL
        "Include eve-tool."
        OFF )
OPTION( EVE_EXTRACT_REV
        "Extract current revision using SVN tools."
        OFF )
OPTION( TINYXML_USE_STL
        "tinyxml will use native STL."
        ON )

#########
# Tests #
#########
# Examine platform
TEST_X64( X64 )
TEST_BIG_ENDIAN( BIG_ENDIAN )

# Check for dependencies
FIND_PACKAGE( "MySql" 5.0 REQUIRED )
FIND_PACKAGE( "ZLIB" REQUIRED )

IF( EVE_EXTRACT_REV )
    FIND_PACKAGE( "Subversion" REQUIRED )
ENDIF( EVE_EXTRACT_REV )

#################
# Configuration #
#################
# Platform-specific configuration
IF( WIN32 )
    # Compile flags
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
    # Standard libraries
    SET( CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} wsock32.lib" )
ENDIF( WIN32 )

# Fill EVEMU_REVISION appropriately
IF( EVE_EXTRACT_REV )
    # Extract current revision
    Subversion_WC_INFO( "${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}" )
    SET( EVEMU_REVISION "${${PROJECT_NAME}_WC_REVISION}" )
ELSE( EVE_EXTRACT_REV )
    # Just fill "N/A"
    SET( EVEMU_REVISION "N/A" )
ENDIF( EVE_EXTRACT_REV )

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
ADD_SUBDIRECTORY( "src/xmlpktgen" )
ADD_SUBDIRECTORY( "src/eve-common" )
ADD_SUBDIRECTORY( "src/eve-server" )

IF( EVE_COLLECTOR )
    ADD_SUBDIRECTORY( "src/eve-collector" )
ENDIF( EVE_COLLECTOR )

IF( EVE_TOOL )
    ADD_SUBDIRECTORY( "src/eve-tool" )
ENDIF( EVE_TOOL )
