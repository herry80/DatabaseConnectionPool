cmake_minimum_required(VERSION 3.8)
project(ConnectionPool)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
set(CMAKE_CXX_STANDARD 17)
include_directories(include/)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)
add_subdirectory(src/)