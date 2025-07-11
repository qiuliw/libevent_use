cmake_minimum_required(VERSION 3.10.0)
project(evsignal VERSION 0.1.0 LANGUAGES C CXX)

add_executable(evsignal main.cpp)

target_link_libraries(evsignal PRIVATE event)