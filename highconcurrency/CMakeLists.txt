cmake_minimum_required(VERSION 3.10.0)
project(highconcurrency VERSION 0.1.0 LANGUAGES C CXX)

add_executable(server main.cpp)

add_executable(client client.cpp)

target_link_libraries(server PRIVATE event)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)