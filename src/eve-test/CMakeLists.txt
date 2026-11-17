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
     "utils/EvilNumberTest.cpp"
     "utils/WeaponFormulaTest.cpp"
     "utils/MovementFormulaTest.cpp"
     "utils/DroneControlRangeTest.cpp"
     "utils/AgentHelpersTest.cpp"
     "utils/CommandPermissionsTest.cpp"
     "utils/MissionLifecycleTest.cpp"
     "utils/ContainerAssemblyTest.cpp"
     "utils/StationOfficeIntegrityTest.cpp"
     "utils/ImageCompatibilityTest.cpp"
     "utils/AsteroidSpawnTest.cpp" )
SET( test_SUPPORT_SOURCE
     "ProfilerTestStub.cpp" )

########################
# Setup the executable #
########################
SOURCE_GROUP( "src"      ${INCLUDE} )
SOURCE_GROUP( "src\\auth"    ${auth_SOURCE} )
SOURCE_GROUP( "src\\marshal" ${marshal_SOURCE} )
SOURCE_GROUP( "src\\utils" FILES ${utils_SOURCE} )

CREATE_TEST_SOURCELIST( TARGET_SOURCELIST "eve-test.cpp"
                        ${auth_SOURCE}
                        ${marshal_SOURCE}
                        ${utils_SOURCE}
                        EXTRA_INCLUDE "eve-test.h" )
ADD_EXECUTABLE( "${TARGET_NAME}"
                ${TARGET_SOURCELIST}
                ${test_SUPPORT_SOURCE} )

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

#########
# Tests #
#########
ADD_TEST( NAME "PasswordModuleTest"
          COMMAND "${TARGET_NAME}" "auth/PasswordModuleTest" )
ADD_TEST( NAME "EVEMarshalTest"
          COMMAND "${TARGET_NAME}" "marshal/EVEMarshalTest" )
ADD_TEST( NAME "EvilNumberTest"
          COMMAND "${TARGET_NAME}" "utils/EvilNumberTest" )
ADD_TEST( NAME "WeaponFormulaTest"
          COMMAND "${TARGET_NAME}" "utils/WeaponFormulaTest" )
ADD_TEST( NAME "MovementFormulaTest"
          COMMAND "${TARGET_NAME}" "utils/MovementFormulaTest" )
ADD_TEST( NAME "DroneControlRangeTest"
          COMMAND "${TARGET_NAME}" "utils/DroneControlRangeTest" )
ADD_TEST( NAME "CommandPermissionsTest"
          COMMAND "${TARGET_NAME}" "utils/CommandPermissionsTest" )
ADD_TEST( NAME "AgentHelpersTest"
          COMMAND "${TARGET_NAME}" "utils/AgentHelpersTest" )
ADD_TEST( NAME "MissionLifecycleTest"
          COMMAND "${TARGET_NAME}" "utils/MissionLifecycleTest" )
ADD_TEST( NAME "ContainerAssemblyTest"
          COMMAND "${TARGET_NAME}" "utils/ContainerAssemblyTest" )
ADD_TEST( NAME "StationOfficeIntegrityTest"
          COMMAND "${TARGET_NAME}" "utils/StationOfficeIntegrityTest" )
ADD_TEST( NAME "ImageCompatibilityTest"
          COMMAND "${TARGET_NAME}" "utils/ImageCompatibilityTest" )
ADD_TEST( NAME "AsteroidSpawnTest"
          COMMAND "${TARGET_NAME}" "utils/AsteroidSpawnTest" )
