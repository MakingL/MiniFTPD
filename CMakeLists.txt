# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.4)

# 项目信息
project(MiniFTPD)

set(CMAKE_CXX_STANDARD 14)

# C++ 11
add_definitions(-std=c++11)

# 指定生成目标
add_executable(MiniFTPD main.cpp tcp.cpp utility.cpp
        epoll_event.cpp signal_wrapper.cpp pipe_wrapper.cpp ftp_handler.cpp command_handle.cpp
        command_handle.h data_handle.cpp)