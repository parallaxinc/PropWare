cmake_minimum_required (VERSION 2.8.4)
project (PropWare)

set(CFLAGS "-Os")

add_executable(${CMAKE_PROJECT_NAME} myFile.cpp)

include(CMake_header.cmake)