cmake_minimum_required (VERSION 2.6.4)

set(MODEL cmm)
project(Simple_${MODEL})
set(CMAKE_PROJECT_NAME Simple_${MODEL})

include(../simpleObjects.cmake)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SIMPLE_OBJECTS})

set(BOARD QUICKSTART)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/PropellerToolchain.cmake)
