cmake_minimum_required (VERSION 2.6.4)

set(MODEL xmm-split)
project(PropWare_${MODEL})
set(CMAKE_PROJECT_NAME PropWare_${MODEL})

add_library(${CMAKE_PROJECT_NAME} STATIC ../PropWare ../spi ../sd)

set(BOARD QUICKSTART)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/CMake_header.cmake)
