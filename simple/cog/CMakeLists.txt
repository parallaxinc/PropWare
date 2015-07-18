cmake_minimum_required (VERSION 3.0.0)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL cog)
include(${PROJECT_SOURCE_DIR}/CMakePropellerSetFlags.cmake)

add_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
install(TARGETS Simple_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware)
