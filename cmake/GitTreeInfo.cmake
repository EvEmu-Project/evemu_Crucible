# - Obtain tree info
# Obtains useful information about a Git tree.
# Expects that git has already been found and
# appropriate variables set up (GIT_EXECUTABLE).
#
# GIT_TREE_INFO( PATH <prefix> )
#   PATH     - path to the Git tree
#   <prefix> - prefix for output variables
#
# Following variables are defined:
#   <prefix>_BRANCH     - name of current branch
#   <prefix>_HASH       - short commit hash
#   <prefix>_TAG_EXACT  - tag of exact current commit
#   <prefix>_TAG_LAST   - last tag relative to current commit
#   <prefix>_ORIGIN_URL - URL of remote origin
#   <prefix>_VERSION    - a nice version string
#

MACRO( GIT_TREE_INFO PATH PREFIX )
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

  # Obtain exact tag
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_TAG_EXACT"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )

  # Obtain last tag
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" describe --tags --abbrev=0
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_TAG_LAST"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )

  # Obtain origin URL
  EXECUTE_PROCESS( COMMAND "${GIT_EXECUTABLE}" config --get remote.origin.url
                   WORKING_DIRECTORY "${PATH}"
                   OUTPUT_VARIABLE "${PREFIX}_ORIGIN_URL"
                   OUTPUT_STRIP_TRAILING_WHITESPACE
                   ERROR_QUIET )

  # Build a nice version string
  IF( "${PREFIX}_TAG_EXACT" )
    # We have an exact tag, use that and nothing else
    SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG_EXACT}" )
  ELSE( "${PREFIX}_TAG_EXACT" )
    # Set version to hash (hash is always available)
    SET( "${PREFIX}_VERSION" "${${PREFIX}_HASH}" )

    # If we have branch name, prepend it
    IF( "${PREFIX}_BRANCH" )
      SET( "${PREFIX}_VERSION" "${${PREFIX}_BRANCH}-${${PREFIX}_VERSION}" )
    ENDIF( "${PREFIX}_BRANCH" )

    # If we have last tag, prepend it
    IF( "${PREFIX}_TAG_LAST" )
      SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG_LAST}-${${PREFIX}_VERSION}" )
    ENDIF( "${PREFIX}_TAG_LAST" )
  ENDIF( "${PREFIX}_TAG_EXACT" )
ENDMACRO( GIT_TREE_INFO )
