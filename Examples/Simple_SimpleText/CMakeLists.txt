cmake_minimum_required (VERSION 2.6.4)

project(SimpleText_Demo)
set(CMAKE_PROJECT_NAME SimpleText_Demo)

add_executable(${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}.c)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/CMake_header.cmake)
