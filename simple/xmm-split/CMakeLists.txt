cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(CFLAGS "-Os")
set(CXXFLAGS )
set(MODEL xmm-split)
include(${PROPWARE_PATH}/CMakeModules/Generic-gcc-Propeller.cmake)

project(Simple_${MODEL})

include(../simpleObjects.cmake)
add_library(${PROJECT_NAME} STATIC ${SIMPLE_OBJECTS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)