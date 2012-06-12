# - detect info using SCM
# Detects if any Source Control Management system (Subversion, Git) is
# in use and pulls information from it if one is found.
#
# DETECT_SCM( DIRECTORY VERSION REPOSITORY )
# - DIRECTORY:  where to search for the SCM
# - VERSION:    where to store current version
# - REPOSITORY: where to store URL of the repository
#

FUNCTION( DETECT_SCM DIRECTORY VERSION REPOSITORY )
  # Include necessary files
  INCLUDE( "GitTreeInfo" )

  # Try Subversion (.svn)
  IF( IS_DIRECTORY "${DIRECTORY}/.svn" )
    FIND_PACKAGE( "Subversion" )

    IF( NOT Subversion_FOUND )
      MESSAGE( WARNING "If you require exact version information, please install Subversion" )
    ELSE()
      Subversion_WC_INFO( "${DIRECTORY}" SVN )
      SET( "${VERSION}"    "r${SVN_WC_LAST_CHANGED_REV}" PARENT_SCOPE )
      SET( "${REPOSITORY}" "${SVN_WC_ROOT}"              PARENT_SCOPE )
    ENDIF( NOT Subversion_FOUND )
  # Try Git (.git)
  ELSEIF( IS_DIRECTORY "${DIRECTORY}/.git" )
    FIND_PACKAGE( "Git" )

    IF( NOT GIT_FOUND )
      MESSAGE( WARNING "If you require exact version information, please install Git" )
    ELSE()
      GIT_TREE_INFO( "${DIRECTORY}" GIT )
      SET( "${VERSION}"    "${GIT_VERSION}"    PARENT_SCOPE )
      SET( "${REPOSITORY}" "${GIT_ORIGIN_URL}" PARENT_SCOPE )
    ENDIF( NOT GIT_FOUND )
  ENDIF( IS_DIRECTORY "${DIRECTORY}/.svn" )
ENDFUNCTION( DETECT_SCM )
