cmake_minimum_required(VERSION 3.10.0)
project(ioev VERSION 0.1.0 LANGUAGES C CXX)

add_executable(receiver receiver.cpp)
# 链接 libevent
target_link_libraries(receiver PRIVATE event)

add_executable(send send.cpp)

# 设置所有可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
