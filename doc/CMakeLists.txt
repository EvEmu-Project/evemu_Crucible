#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME       "doc" )
SET( TARGET_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${TARGET_NAME}" )
SET( TARGET_BINARY_DIR "${PROJECT_BINARY_DIR}/${TARGET_NAME}" )

#########
# Files #
#########
SET( DOXYGEN_TEMPLATE "${TARGET_SOURCE_DIR}/Doxyfile.in" )
SET( DOXYGEN_INPUT    "${TARGET_BINARY_DIR}/Doxyfile" )
SET( DOXYGEN_OUTPUT   "${TARGET_BINARY_DIR}/html" )

CONFIGURE_FILE( "${DOXYGEN_TEMPLATE}"
                "${DOXYGEN_INPUT}"
                ESCAPE_QUOTES @ONLY )

########################
# Setup the generation #
########################
ADD_CUSTOM_COMMAND( OUTPUT ${DOXYGEN_OUTPUT}
                    COMMAND "${DOXYGEN_EXECUTABLE}" ARGS "${DOXYGEN_INPUT}"
                    DEPENDS "${DOXYGEN_INPUT}"
                    COMMENT "Generating documentation ..."
                    VERBATIM )

ADD_CUSTOM_TARGET( "${TARGET_NAME}" ALL
                   DEPENDS ${DOXYGEN_OUTPUT}
                   SOURCES "${DOXYGEN_TEMPLATE}" )

INSTALL( DIRECTORY ${DOXYGEN_OUTPUT}
         DESTINATION "doc/${PROJECT_NAME}-${PROJECT_VERSION}" )
