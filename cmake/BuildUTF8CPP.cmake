# - Build config of UTF8-CPP
# Defines several variables needed for automatic
# download and build of source archive.
#

# Search for the package
FIND_PACKAGE( "UTF8CPP" )
SET( PKG_FOUND TRUE )

# 设置包含目录
SET( UTF8CPP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/DEP/utfcpp-4.0.6/source" )
