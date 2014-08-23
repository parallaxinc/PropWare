cmake_minimum_required (VERSION 3.0.0)

if (NOT DEFINED PROPWARE_PATH)
    file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)
endif ()

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL cog)
include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

project(PropWare_${MODEL})

add_library(${PROJECT_NAME} STATIC ../PropWare.cpp)
