#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET(TARGET_NAME "eve-db")
SET(TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}")
SET(TARGET_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}")

IF (ODB_LIBODB_FOUND)
    ADD_COMPILE_DEFINITIONS(
            HAVE_ODB_H
    )
    IF (ODB_mysql_FOUND)
        ADD_COMPILE_DEFINITIONS(HAVE_ODB_MYSQL_H)
    ENDIF (ODB_mysql_FOUND)
    IF (ODB_pgsql_FOUND)
        ADD_COMPILE_DEFINITIONS(HAVE_ODB_PGSQL_H)
    ENDIF (ODB_pgsql_FOUND)
ENDIF (ODB_LIBODB_FOUND)

INCLUDE(${ODB_USE_FILE})
SET(ODB_COMPILE_DEBUG TRUE)
FILE(GLOB_RECURSE ODB_MODELS models/**/*.h)
FOREACH (__ODB_MODEL_FILE ${ODB_MODELS})
    CMAKE_PATH(GET __ODB_MODEL_FILE PARENT_PATH __ODB_MODEL_DIRNAME)
    LIST(APPEND ODB_MODELS_DIRS ${__ODB_MODEL_DIRNAME})
ENDFOREACH ()
LIST(REMOVE_DUPLICATES ODB_MODELS_DIRS)
SET(ODB_COMPILE_HEADER_SUFFIX ".hxx")
SET(ODB_COMPILE_INLINE_SUFFIX ".ixx")
ODB_COMPILE(
        ODB_MODEL_SOURCES
        FILES ${ODB_MODELS}
        DB ${ODB_COMPONENTS}
        MULTI_DATABASE dynamic
        GENERATE_QUERY
        GENERATE_SESSION
        GENERATE_SCHEMA
        GENERATE_PREPARED
        STANDARD c++11
)
MESSAGE(STATUS "ODB models compiled successfully")

#########
# Files #
#########
SET(INCLUDE
        "${TARGET_INCLUDE_DIR}/models/invType/InvType.h"
        "${TARGET_INCLUDE_DIR}/../eve-core/utils/Singleton.h"
        "${TARGET_INCLUDE_DIR}/EVEORMCore.h"
        )
SET(SOURCE
        "${TARGET_SOURCE_DIR}/EVEORMCore.cpp"
        "${TARGET_INCLUDE_DIR}/models/invType/InvType.h"
        )


#####################
# Setup the library #
#####################

ADD_LIBRARY(
        "${TARGET_NAME}"
        SHARED
        ${INCLUDE}
        ${SOURCE}
        ${ODB_MODEL_SOURCES}
        ${ODB_MODELS}
)

TARGET_INCLUDE_DIRECTORIES(
        "${TARGET_NAME}"
        "${TARGET_INCLUDE_DIR}"
        "${ODB_INCLUDE_DIRS}"
        "${ODB_COMPILE_OUTPUT_DIR}"
        ${ODB_MODELS_DIRS}
        ${TARGET_INCLUDE_DIR}/../eve-core
        ${MYSQL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(
        "${TARGET_NAME}"
        ${MYSQL_LIBRARIES}
        ${ODB_LIBRARIES}
)
