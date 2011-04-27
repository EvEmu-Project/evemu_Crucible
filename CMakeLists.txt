#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

###############
# CMake setup #
###############
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 FATAL_ERROR )

PROJECT( "evemu" )
SET( PROJECT_DESCRIPTION_SUMMARY "A server emulator for EVE Online" )
SET( PROJECT_VENDOR              "The EVEmu development team" )
SET( PROJECT_VERSION             "0.7-experimental" )

SET( CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake"
     "${CMAKE_MODULE_PATH}" )

INCLUDE( "CheckSymbolExists" )

INCLUDE( "TargetBuildPCH" )
INCLUDE( "TestX64" )

###########
# Options #
###########
SET( EVEMU_COLLECTOR_ENABLE OFF
     CACHE BOOL   "Build eve-collector." )
SET( EVEMU_DOC_ENABLE       OFF
     CACHE BOOL   "Generate HTML documentation (requires Doxygen)." )
SET( EVEMU_SOURCE_SCM       ""
     CACHE STRING "Source SCM in use; valid values: '', 'svn', 'git'" )
SET( EVEMU_TOOL_ENABLE      OFF
     CACHE BOOL   "Build eve-tool." )

SET( EVEMU_ROOT_DIR "..////"
     CACHE PATH "The root of EVEmu workspace." )
MARK_AS_ADVANCED( EVEMU_ROOT_DIR )

SET( TINYXML_USE_STL ON
     CACHE BOOL "tinyxml will use native STL." )
MARK_AS_ADVANCED( TINYXML_USE_STL )

#########
# Tests #
#########
TEST_X64( X64 )

CHECK_SYMBOL_EXISTS( "__FreeBSD__" "" FREE_BSD )

SET( GNUC ${CMAKE_COMPILER_IS_GNUCXX} )

################
# Dependencies #
################
FIND_PACKAGE( "Threads" REQUIRED )
FIND_PACKAGE( "MySQL" 5.0 REQUIRED )
FIND_PACKAGE( "ZLIB" REQUIRED )

IF( EVEMU_DOC_ENABLE )
    FIND_PACKAGE( "Doxygen" REQUIRED )
ENDIF( EVEMU_DOC_ENABLE )

IF( NOT EVEMU_SOURCE_REVISION )
    IF( EVEMU_SOURCE_SCM STREQUAL "svn" )
        FIND_PACKAGE( "Subversion" REQUIRED )
    ELSEIF( EVEMU_SOURCE_SCM STREQUAL "git" )
        FIND_PACKAGE( "Git" REQUIRED )
    ENDIF( EVEMU_SOURCE_SCM STREQUAL "svn" )
ENDIF( NOT EVEMU_SOURCE_REVISION )

FIND_PACKAGE( "Gangsta" QUIET )
FIND_PACKAGE( "TinyXML" QUIET )
FIND_PACKAGE( "UTF8CPP" QUIET )

#################
# Configuration #
#################
IF( WIN32 )
    SET( PROJECT_STANDARD_LIBRARIES "wsock32" ${CMAKE_THREAD_LIBS_INIT} )
ELSE( WIN32 )
    SET( PROJECT_STANDARD_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} )
ENDIF( WIN32 )

IF( MSVC )
    #
    # This is here as we usually use pre-built MySQL binaries which are
    # compiled with /MT (use static libc); building with /MD (use DLL libc)
    # results in linker warning during the linking stage, however the resultant
    # binary works and is even a little bit smaller due to the dynamic linking.
    #
    # For now we supress the linker warning by forcing static libc.
    #
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
    STRING( REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
ENDIF( MSVC )

IF( NOT EVEMU_SOURCE_REVISION )
    IF( EVEMU_SOURCE_SCM STREQUAL "svn" )
        Subversion_WC_INFO( "${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}" )

        SET( EVEMU_SOURCE_REVISION "r${${PROJECT_NAME}_WC_LAST_CHANGED_REV}"
             CACHE STRING "Revision of the source." )
        MARK_AS_ADVANCED( EVEMU_SOURCE_REVISION )
    ELSEIF( EVEMU_SOURCE_SCM STREQUAL "git" )
        GIT_TREE_INFO( "${PROJECT_SOURCE_DIR}" "${PROJECT_NAME}" )

        SET( EVEMU_SOURCE_REVISION "${${PROJECT_NAME}_HASH_SHORT}"
             CACHE STRING "Revision of the source." )
        MARK_AS_ADVANCED( EVEMU_SOURCE_REVISION )
    ENDIF( EVEMU_SOURCE_SCM STREQUAL "svn" )
ENDIF( NOT EVEMU_SOURCE_REVISION )

IF( EVEMU_SOURCE_REVISION )
    SET( PROJECT_VERSION "${PROJECT_VERSION}-${EVEMU_SOURCE_REVISION}" )
ENDIF( EVEMU_SOURCE_REVISION )

############
# config.h #
############
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/config.h.in"
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
IF( EVEMU_DOC_ENABLE )
    ADD_SUBDIRECTORY( "doc" )
ENDIF( EVEMU_DOC_ENABLE )

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

################
# EVEmu source #
################
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

###############
# CPack setup #
###############
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION_SUMMARY}" )
SET( CPACK_PACKAGE_VENDOR              "${PROJECT_VENDOR}" )
SET( CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}" )

INCLUDE( "CPack" )
