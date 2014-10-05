cmake_minimum_required (VERSION 3.0.0)

include(./CMakePropellerHeader.cmake)
set(CMAKE_BUILD_TYPE None)

project(PropWare C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT)

set(PROPWARE_MAIN_PACKAGE 1)

#add_subdirectory(simple)
#add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)
