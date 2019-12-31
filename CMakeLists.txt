# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.4)

# 项目信息
project(MiniFTPD)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找指定目录下的所有源文件，
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(src/ DIR_SRCS)

# 指定生成目标
add_executable(MiniFTPD ${DIR_SRCS})

# 添加链接库
target_link_libraries(MiniFTPD crypt)