#
# CMake build system file for EVEmu.
#
# Author: Bloody.Rabbit
#

##############
# Initialize #
##############
SET( TARGET_NAME        "mysql" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}/include" )
SET( TARGET_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/dep/${TARGET_NAME}/lib" )

SET( TARGET_INCLUDE_DIRS
     "${TARGET_INCLUDE_DIR}" )

# Export directories
SET( ${TARGET_NAME}_INCLUDE_DIRS
     ${TARGET_INCLUDE_DIRS}
     PARENT_SCOPE )
SET( ${TARGET_NAME}_LIBRARIES
     "${TARGET_LIBRARY_DIR}/opt/libmysql.lib"
     PARENT_SCOPE )

#########
# Files #
#########
SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/config-win.h"
     "${TARGET_INCLUDE_DIR}/decimal.h"
     "${TARGET_INCLUDE_DIR}/errmsg.h"
     "${TARGET_INCLUDE_DIR}/keycache.h"
     "${TARGET_INCLUDE_DIR}/m_ctype.h"
     "${TARGET_INCLUDE_DIR}/m_string.h"
     "${TARGET_INCLUDE_DIR}/my_alloc.h"
     "${TARGET_INCLUDE_DIR}/my_attribute.h"
     "${TARGET_INCLUDE_DIR}/my_compiler.h"
     "${TARGET_INCLUDE_DIR}/my_dbug.h"
     "${TARGET_INCLUDE_DIR}/my_dir.h"
     "${TARGET_INCLUDE_DIR}/my_getopt.h"
     "${TARGET_INCLUDE_DIR}/my_global.h"
     "${TARGET_INCLUDE_DIR}/my_list.h"
     "${TARGET_INCLUDE_DIR}/my_net.h"
     "${TARGET_INCLUDE_DIR}/my_no_pthread.h"
     "${TARGET_INCLUDE_DIR}/my_pthread.h"
     "${TARGET_INCLUDE_DIR}/mysql_com.h"
     "${TARGET_INCLUDE_DIR}/mysqld_ername.h"
     "${TARGET_INCLUDE_DIR}/mysqld_error.h"
     "${TARGET_INCLUDE_DIR}/mysql_embed.h"
     "${TARGET_INCLUDE_DIR}/mysql.h"
     "${TARGET_INCLUDE_DIR}/mysql_time.h"
     "${TARGET_INCLUDE_DIR}/mysql_version.h"
     "${TARGET_INCLUDE_DIR}/my_sys.h"
     "${TARGET_INCLUDE_DIR}/my_xml.h"
     "${TARGET_INCLUDE_DIR}/sql_common.h"
     "${TARGET_INCLUDE_DIR}/sql_state.h"
     "${TARGET_INCLUDE_DIR}/sslopt-case.h"
     "${TARGET_INCLUDE_DIR}/sslopt-longopts.h"
     "${TARGET_INCLUDE_DIR}/sslopt-vars.h"
     "${TARGET_INCLUDE_DIR}/typelib.h" )
SET( SOURCE
     "" )

SET( mysql_INCLUDE
     "${TARGET_INCLUDE_DIR}/mysql/plugin.h" )
SET( mysql_SOURCE
     "" )

#####################
# Setup the library #
#####################
INCLUDE_DIRECTORIES( ${TARGET_INCLUDE_DIRS} )

SOURCE_GROUP( "include"        FILES ${INCLUDE} )
SOURCE_GROUP( "include\\mysql" FILES ${mysql_INCLUDE} )

SOURCE_GROUP( "src"        FILES ${SOURCE} )
SOURCE_GROUP( "src\\mysql" FILES ${mysql_SOURCE} )

ADD_CUSTOM_TARGET( "${TARGET_NAME}"
                   SOURCES ${INCLUDE}       ${SOURCE}
                           ${mysql_INCLUDE} ${mysql_SOURCE} )
