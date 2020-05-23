# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.4)

# 项目信息
project(MiniFTPD)

# 设置版本号
set (MiniFTPD_VERSION_MAJOR 0)
set (MiniFTPD_VERSION_MINOR 1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CXX_FLAGS
        -g
        -DCHECK_PTHREAD_RETURN_VALUE
        -D_FILE_OFFSET_BITS=64
        -std=c++11
        )
if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND CXX_FLAGS "-Wno-null-dereference")
    list(APPEND CXX_FLAGS "-Wno-sign-conversion")
    list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
    list(APPEND CXX_FLAGS "-Wthread-safety")
    list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

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

include_directories(${INC_DIR}) # 头文件
add_subdirectory(lib/yaml-cpp)

# 指定生成目标
add_executable(MiniFTPD ${DIR_SRCS})

# 添加链接库
target_link_libraries(MiniFTPD crypt)
target_link_libraries(MiniFTPD yaml-cpp)
