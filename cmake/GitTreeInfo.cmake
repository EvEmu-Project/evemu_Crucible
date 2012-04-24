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
#   <prefix>_BRANCH    - name of current branch
#   <prefix>_HASH      - short commit hash
#   <prefix>_TAG_EXACT - tag of exact current commit
#   <prefix>_TAG_LAST  - last tag relative to current commit
#   <prefix>_VERSION   - a nice version string
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

  # Build a nice version string
  IF( "${PREFIX}_TAG_EXACT" )
    # We have an exact tag, use that and nothing else
    SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG_EXACT}" )
  ELSEIF( "${PREFIX}_TAG_LAST" )
    IF( "${PREFIX}_BRANCH" )
      # We have the last tag and a branch, use tag + branch + hash
      SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG_LAST}-${${PREFIX}_BRANCH}-${${PREFIX}_HASH}" )
    ELSE( "${PREFIX}_BRANCH" )
      # We have the last tag but no branch, use tag + hash
      SET( "${PREFIX}_VERSION" "${${PREFIX}_TAG_LAST}-${${PREFIX}_HASH}" )
    ENDIF( "${PREFIX}_BRANCH" )
  ELSEIF( "${PREFIX}_BRANCH" )
    # No last tag but we have a branch, use branch + hash
    SET( "${PREFIX}_VERSION" "${${PREFIX}_BRANCH}-${${PREFIX}_HASH}" )
  ELSE( "${PREFIX}_TAG_EXACT" )
    # No last tag and no branch, use only hash
    SET( "${PREFIX}_VERSION" "${${PREFIX}_HASH}" )
  ENDIF( "${PREFIX}_TAG_EXACT" )
ENDMACRO( GIT_TREE_INFO )
