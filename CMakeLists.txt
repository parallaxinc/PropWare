cmake_minimum_required (VERSION 2.6.4)

set(CMAKE_TOOLCHAIN_FILE ./PropellerToolchain.cmake)
set (CMAKE_BUILD_TYPE None)
project(PropWare C CXX ASM COGC COGCXX ECOGC ECOGCXX)

set(PROPWARE_MAIN_PACKAGE 1)

add_subdirectory(simple)
add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)
