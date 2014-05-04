cmake_minimum_required (VERSION 2.6.4)

project(PropWare_Demo)
set(CMAKE_PROJECT_NAME PropWare_Demo)

add_executable(${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.cpp)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/CMake_header.cmake)
