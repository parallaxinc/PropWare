cmake_minimum_required (VERSION 3.0.0)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
    file(TO_NATIVE_PATH PROPWARE_PATH ${PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL lmm)
include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

project(Simple_${MODEL})

include(../simpleObjects.cmake)
add_library(${PROJECT_NAME} STATIC ${SIMPLE_OBJECTS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
