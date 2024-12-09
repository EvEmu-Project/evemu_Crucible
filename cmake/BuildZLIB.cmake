# - Build config of zlib
# Defines several variables needed for automatic
# download and build of source archive.
#

# Version 1.2.3 (July 2005) eliminates potential security
# vulnerabilities in zlib 1.2.1 and 1.2.2, so all users of those
# versions should upgrade immediately.
FIND_PACKAGE( "ZLIB" 1.2.3 )
SET( PKG_FOUND TRUE )  # 直接设置为找到，使用本地文件

# 设置策略以解决文件扩展名警告
if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)
endif()

# 首先配置 zconf.h
configure_file(
    "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/zconf.h.in"
    "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/zconf.h"
    COPYONLY
)

# 设置包含目录和库
SET( ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7" )
SET( ZLIB_LIBRARY "zlib" )

# 添加库（只包含源文件，不包含头文件）
ADD_LIBRARY(
  "zlib"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/adler32.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/compress.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/crc32.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/deflate.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/gzclose.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/gzlib.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/gzread.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/gzwrite.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/infback.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/inffast.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/inflate.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/inftrees.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/trees.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/uncompr.c"
  "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/zutil.c"
)

# 设置包含目录
target_include_directories(zlib PUBLIC "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7")

# 确保 zconf.h 在编译前生成
add_custom_command(
    TARGET zlib
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/zconf.h.in"
        "${PROJECT_SOURCE_DIR}/DEP/zlib-1.2.7/zconf.h"
)

if(NOT WIN32)
  # Linux 下可能需要的额外编译选项
  target_compile_options(zlib PRIVATE -fPIC)
endif()
