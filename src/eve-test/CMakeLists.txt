#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "eve-test" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/eve-test.h" )
# No eve-test.cpp, generated on the fly.

# You must NOT use TARGET_SOURCE_DIR (or, to be
# exact, use absolute paths) when specifying
# the test sources.
SET( auth_SOURCE
     "auth/PasswordModuleTest.cpp" )
SET( marshal_SOURCE
     "marshal/EVEMarshalTest.cpp" )
SET( utils_SOURCE
     "utils/EvilNumberTest.cpp" )
SET( network_SOURCE
     "network/ProtocolSafetyTest.cpp" )
 SET( server_SOURCE
       "server/ServerLoopTimingTest.cpp"
       "server/ServerShutdownTest.cpp"
       "server/MovementCommandTest.cpp"
       "server/DestinyBinDumpTest.cpp" )
SET( SUPPORT_SOURCE
     "ProfilerStub.cpp" )

########################
# Setup the executable #
########################
SOURCE_GROUP( "src"      ${INCLUDE} )
SOURCE_GROUP( "src\\auth"    ${auth_SOURCE} )
SOURCE_GROUP( "src\\marshal" ${marshal_SOURCE} )
SOURCE_GROUP( "src\\utils"   ${utils_SOURCE} )
SOURCE_GROUP( "src\\network" ${network_SOURCE} )
SOURCE_GROUP( "src\\server"  ${server_SOURCE} )
SOURCE_GROUP( "support"       ${SUPPORT_SOURCE} )

CREATE_TEST_SOURCELIST( TARGET_SOURCELIST "eve-test.cpp"
                        ${auth_SOURCE}
                         ${marshal_SOURCE}
                         ${utils_SOURCE}
                         ${network_SOURCE}
                         ${server_SOURCE}
                         EXTRA_INCLUDE "eve-test.h" )
ADD_EXECUTABLE( "${TARGET_NAME}"
                 ${TARGET_SOURCELIST}
                 ${SUPPORT_SOURCE} )

#TARGET_BUILD_PCH( "${TARGET_NAME}"
#                  "${TARGET_INCLUDE_DIR}/eve-test.h"
#                  "${CMAKE_CURRENT_BINARY_DIR}/eve-test.cpp" )
target_precompile_headers( "${TARGET_NAME}" PUBLIC
                  "${TARGET_INCLUDE_DIR}/eve-test.h" )
TARGET_INCLUDE_DIRECTORIES( "${TARGET_NAME}"
                             ${eve-common_INCLUDE_DIRS}
                             "${TARGET_INCLUDE_DIR}"
                             "${PROJECT_SOURCE_DIR}/src/eve-server" )
TARGET_LINK_LIBRARIES( "${TARGET_NAME}"
                       "eve-common" )

IF ( WIN32 )
  IF ( TARGET mysql )
    SET( RUNTIME_DLLS "$<TARGET_FILE:mysql>" )
  ELSE()
    GET_FILENAME_COMPONENT( MYSQL_LIBRARY_DIR
                            "${MYSQL_LIBRARIES}" DIRECTORY )
    SET( RUNTIME_DLLS
         "${MYSQL_LIBRARY_DIR}/libmariadb.dll" )
  ENDIF()

  IF ( CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW" )
    GET_FILENAME_COMPONENT( LLVM_MINGW_BIN
                            "${CMAKE_CXX_COMPILER}" DIRECTORY )
    LIST( APPEND RUNTIME_DLLS
          "${LLVM_MINGW_BIN}/libc++.dll"
          "${LLVM_MINGW_BIN}/libunwind.dll" )
  ENDIF()

  ADD_CUSTOM_COMMAND(
    TARGET "${TARGET_NAME}" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            ${RUNTIME_DLLS}
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    VERBATIM )
ENDIF()

#########
# Tests #
#########
ADD_TEST( NAME "PasswordModuleTest"
          COMMAND "${TARGET_NAME}" "auth/PasswordModuleTest" )
ADD_TEST( NAME "EVEMarshalTest"
          COMMAND "${TARGET_NAME}" "marshal/EVEMarshalTest" )
ADD_TEST( NAME "EvilNumberTest"
          COMMAND "${TARGET_NAME}" "utils/EvilNumberTest" )
ADD_TEST( NAME "ProtocolSafetyTest"
          COMMAND "${TARGET_NAME}" "network/ProtocolSafetyTest" )
ADD_TEST( NAME "ServerLoopTimingTest"
          COMMAND "${TARGET_NAME}" "server/ServerLoopTimingTest" )
 ADD_TEST( NAME "ServerShutdownTest"
           COMMAND "${TARGET_NAME}" "server/ServerShutdownTest" )
  ADD_TEST( NAME "MovementCommandTest"
            COMMAND "${TARGET_NAME}" "server/MovementCommandTest" )
  ADD_TEST( NAME "DestinyBinDumpTest"
            COMMAND "${TARGET_NAME}" "server/DestinyBinDumpTest" )
