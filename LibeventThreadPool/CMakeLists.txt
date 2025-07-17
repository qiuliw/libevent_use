cmake_minimum_required(VERSION 3.10)

project(LibeventThreadPool)

set(CMAKE_CXX_STANDARD 17)

# 包含libevent头文件
include_directories(${LIBEVENT_INCLUDE_DIRS})

aux_source_directory(. SRC_LIST)
# 添加可执行文件
add_executable(LibeventThreadPool ${SRC_LIST})

# 链接libevent库
target_link_libraries(LibeventThreadPool PRIVATE event)

# 添加线程库
find_package(Threads REQUIRED)

# 链接线程库
target_link_libraries(LibeventThreadPool PRIVATE Threads::Threads)