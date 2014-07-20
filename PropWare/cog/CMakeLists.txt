cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(CFLAGS "-Os")
set(CXXFLAGS )
set(MODEL cog)
include(${PROPWARE_PATH}/Generic-gcc-Propeller.cmake)

project(PropWare_${MODEL})

add_library(${PROJECT_NAME} STATIC ../PropWare.cpp)
