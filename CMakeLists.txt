cmake_minimum_required(VERSION 3.3)

set(PROPWARE_MAIN_PACKAGE TRUE)

# Get the PropWare version
file(READ "${CMAKE_CURRENT_LIST_DIR}/version.txt" VERSION)
string(STRIP ${VERSION} VERSION)

if (DEFINED ENV{BUILD_NUMBER} AND NOT DEFINED ENV{PROPWARE_RELEASE})
    set(VERSION "${VERSION}.$ENV{BUILD_NUMBER}")
endif ()

# Load special modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules")
find_package(PropWare REQUIRED)

# Allow using `if (VAR IN_LIST MY_LIST)`. Requires CMake 3.3
cmake_policy(SET CMP0057 NEW)

# Set the project, version number and all languages used
project(PropWare
    VERSION ${VERSION}
    LANGUAGES C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT SPIN2DAT)

add_subdirectory(external_libs) # Make sure this is first since it modifies the include directories needed later
add_subdirectory(PropWare)
add_subdirectory(Examples)
add_subdirectory(test)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(docs
        "${DOXYGEN_EXECUTABLE}" Doxyfile
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

option(LINUX_PACKAGE "Create complete package for Linux" OFF)
option(WIN32_PACKAGE "Create complete package for Windows" OFF)
option(OSX_PACKAGE "Create complete package for Mac OSX" OFF)
option(PI2_PACKAGE "Create complete package for Raspberry Pi 2" OFF)

include(ExternalProject)
include(./DownloadCMakeBinaries.cmake)
include(./DownloadSpin2cpp.cmake)

include(./CMakePropWareInstall.cmake)
include(./CMakeCPack.cmake)
