cmake_minimum_required(VERSION 3.12)

set(PROPWARE_MAIN_PACKAGE TRUE)
set(CMAKE_ERROR_DEPRECATED TRUE)

# Get the PropWare version
file(READ "${CMAKE_CURRENT_LIST_DIR}/version.txt" VERSION)
string(STRIP ${VERSION} VERSION)

if (DEFINED ENV{BUILD_NUMBER} AND NOT DEFINED ENV{PROPWARE_RELEASE})
    set(VERSION "${VERSION}.$ENV{BUILD_NUMBER}")
endif ()

# Load special modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules")
set(SAVE_TEMPS OFF CACHE BOOL "Must be turned off to prevent build failures in Simple during parallel compilation.")
find_package(PropWare REQUIRED)

# Set the project, version number and all languages used
project(PropWare
    VERSION ${VERSION}
    LANGUAGES C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT SPIN2DAT)

option(PACKAGE_LINUX "Create complete package for Linux" OFF)
option(PACKAGE_WIN32 "Create complete package for Windows" OFF)
option(PACKAGE_OSX "Create complete package for Mac OSX" OFF)

include(./PropWareSources.cmake)
include(./DownloadWindowsBinaries.cmake)
include(./PropWareInstall.cmake)
include(./PropWareCPack.cmake)
include(./DocumentationConfig.cmake)
