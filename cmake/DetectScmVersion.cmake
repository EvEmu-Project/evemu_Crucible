# - detect version using SCM
# Detects if any Source Control Management system (Subversion, Git) is
# in use and pulls current source version from it if one is found.
#
# DETECT_SCM_VERSION( DIRECTORY VARIABLE )
# - DIRECTORY: where to search for the SCM
# - VARIABLE: where to store the result
#

FUNCTION( DETECT_SCM_VERSION DIRECTORY VARIABLE )
  # Include necessary files
  INCLUDE( "GitTreeInfo" )

  # Try Subversion (.svn)
  IF( IS_DIRECTORY "${DIRECTORY}/.svn" )
    FIND_PACKAGE( "Subversion" )

    IF( NOT Subversion_FOUND )
      MESSAGE( STATUS "WARNING: Cannot use present Subversion version information" )
    ELSE()
      Subversion_WC_INFO( "${DIRECTORY}" SVN )
      SET( "${VARIABLE}" "r${SVN_WC_LAST_CHANGED_REV}" PARENT_SCOPE )
    ENDIF( NOT Subversion_FOUND )
  # Try Git (.git)
  ELSEIF( IS_DIRECTORY "${DIRECTORY}/.git" )
    FIND_PACKAGE( "Git" )

    IF( NOT GIT_FOUND )
      MESSAGE( STATUS "WARNING: Cannot use present Git version information" )
    ELSE()
      GIT_TREE_INFO( "${DIRECTORY}" GIT )
      SET( "${VARIABLE}" "${GIT_VERSION}" PARENT_SCOPE )
    ENDIF( NOT GIT_FOUND )
  ENDIF( IS_DIRECTORY "${DIRECTORY}/.svn" )
ENDFUNCTION( DETECT_SCM_VERSION VARIABLE )
