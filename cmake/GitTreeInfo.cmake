# - Obtain tree info
# Obtains useful information about a Git tree.
#
# GIT_TREE_INFO( PATH <prefix> )
#   PATH     - path to the Git tree
#   <prefix> - prefix for output variables
#
# Following variables are defined:
#   <prefix>_BRANCH  - name of current branch (may be empty)
#   <prefix>_HASH    - short commit hash
#   <prefix>_TAG     - current tag (may be empty)
#   <prefix>_VERSION - a nice version string
#

MACRO( GIT_TREE_INFO PATH PREFIX )
  # Find required Git package
  FIND_PACKAGE( "Git" REQUIRED )

  # Obtain branch name
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" symbolic-ref HEAD
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_BRANCH"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )
  GET_FILENAME_COMPONENT( "${PREFIX}_BRANCH" "${${PREFIX}_BRANCH}" NAME )

  # Obtain hash
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_HASH"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )

  # Obtain tag
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_TAG"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )

  # Build a nice version string
  IF( "${PREFIX}_TAG" )
    SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG}" )
  ELSEIF( NOT "${PREFIX}_BRANCH" )
    SET( "${PREFIX}_VERSION" "${${PREFIX}_HASH}" )
  ELSE( "${PREFIX}_TAG" )
    SET( "${PREFIX}_VERSION" "${${PREFIX}_BRANCH}-${${PREFIX}_HASH}" )
  ENDIF( "${PREFIX}_TAG" )
ENDMACRO( GIT_TREE_INFO )
