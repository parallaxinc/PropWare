cmake_minimum_required (VERSION 3.0.0)

if (DEFINED PROPWARE_PATH)
    set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)
else ()
    set(CMAKE_TOOLCHAIN_FILE $ENV{PROPWARE_PATH}/PropellerToolchain.cmake)
endif ()
include(./CMakePropellerDefOptions.cmake)
set(CMAKE_BUILD_TYPE None)

set(PROPWARE_PRINT_FLOAT ON)

project(PropWare C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT)

set(PROPWARE_MAIN_PACKAGE 1)

add_subdirectory(simple)
add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)

add_custom_target(docs
    cd ${CMAKE_SOURCE_DIR} && doxygen Doxyfile)

include(./CMakePropWareInstall.cmake)
include(./CMakePropWarePackage.cmake)
