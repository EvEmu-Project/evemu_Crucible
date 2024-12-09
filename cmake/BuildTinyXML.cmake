# - Build config of TinyXML
# Defines several variables needed for automatic
# download and build of source archive.
#

# We need "TiXmlNode::TINYXML_ELEMENT" which is present
# in TinyXML 2.6.0 and later.
FIND_PACKAGE( "TinyXML" 2.6.0 )
SET( PKG_FOUND TRUE )  # 直接设置为找到，使用本地文件

# 设置包含目录和库
SET( TINYXML_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/DEP/tinyxml" )
SET( TINYXML_LIBRARIES "tinyxml" )

# 如果启用 STL
IF( TIXML_USE_STL )
  ADD_DEFINITIONS( "-DTIXML_USE_STL" )
ENDIF( TIXML_USE_STL )

# 添加库
ADD_LIBRARY(
  "tinyxml"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinystr.h"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinystr.cpp"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinyxml.h"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinyxml.cpp"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinyxmlerror.cpp"
  "${PROJECT_SOURCE_DIR}/DEP/tinyxml/tinyxmlparser.cpp"
)
