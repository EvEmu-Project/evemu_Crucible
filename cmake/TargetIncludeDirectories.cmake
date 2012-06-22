# - Define a macro to handle include dirs of a target
#
# TARGET_INCLUDE_DIRECTORIES( TARGET [..directories..] )
#     - Includes all listed directories
#     - Exports a variable called <TARGET>_INCLUDE_DIRS
#       listing all directories.
#
#     Arguments:
#         TARGET - name of the target
#

MACRO( TARGET_INCLUDE_DIRECTORIES TARGET )
  # Add the include directories
  SET_PROPERTY(
    TARGET "${TARGET}" APPEND
    PROPERTY INCLUDE_DIRECTORIES ${ARGN}
    )
  # Export a variable with the list of directories
  SET(
    "${TARGET}_INCLUDE_DIRS"
    ${ARGN} PARENT_SCOPE
    )
ENDMACRO( TARGET_INCLUDE_DIRECTORIES )
