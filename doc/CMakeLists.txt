#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

SET( DOXYGEN_OUTPUT
  "${CMAKE_CURRENT_BINARY_DIR}/html"
  )

CONFIGURE_FILE(
  "Doxyfile.in"
  "Doxyfile"
  ESCAPE_QUOTES @ONLY
  )
ADD_CUSTOM_COMMAND(
  OUTPUT ${DOXYGEN_OUTPUT}
  COMMAND "${DOXYGEN_EXECUTABLE}" ARGS "Doxyfile"
  DEPENDS "Doxyfile"
  COMMENT "Generating documentation ..."
  VERBATIM
  )
ADD_CUSTOM_TARGET(
  "doc" ALL
  DEPENDS ${DOXYGEN_OUTPUT}
  SOURCES "Doxyfile.in"
  )

INSTALL(
  DIRECTORY ${DOXYGEN_OUTPUT}
  DESTINATION "share/doc/${PROJECT_NAME}"
  )
