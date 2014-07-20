cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(CFLAGS "-Os")
set(CXXFLAGS )
set(MODEL lmm)
include(${PROPWARE_PATH}/Generic-gcc-Propeller.cmake)

project(Simple_${MODEL})

include(../simpleObjects.cmake)
add_library(${PROJECT_NAME} STATIC ${SIMPLE_OBJECTS})
