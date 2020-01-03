# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.4)

# 项目信息
project(MiniFTPD)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找指定目录下的所有源文件，
# 并将名称保存到 DIR_SRCS 变量
set(SEC_DIR src/)    # 源文件所在目录
aux_source_directory(${SEC_DIR} DIR_SRCS)

# 设置配置文件所在的路径
set(CONFIGURE_FILE_PATH \"config.yaml\")
configure_file (
        "${SEC_DIR}/config.h.in"
        "${SEC_DIR}/config.h"
)

# 是否为 Debug 模式（是否输出 Debug 信息）
 option (DEBUG "Use debug mode" ON)
 IF (NOT ${DEBUG})
     ADD_DEFINITIONS(-DNDEBUG)
 ENDIF()

set(INC_DIR include)    # 头文件
set(LINK_DIR lib/)  # 链接库的位置

include_directories(${INC_DIR}) # 头文件
link_directories(${LINK_DIR}) # 链接库的位置

# 指定生成目标
add_executable(MiniFTPD ${DIR_SRCS})

# 添加链接库
target_link_libraries(MiniFTPD crypt)
target_link_libraries(MiniFTPD libyaml-cpp.a)
