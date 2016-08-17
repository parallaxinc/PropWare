cmake_minimum_required(VERSION 3.3)

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
find_package(PropWare REQUIRED)

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
    set(TEAMCITY_BUILD_ID "$ENV{TEAMCITY_BUILD_ID}")
    configure_file("${PROJECT_SOURCE_DIR}/docs/Download.md.in"
        "${PROJECT_SOURCE_DIR}/docs/Download.md"
        @ONLY)
    configure_file("${PROJECT_SOURCE_DIR}/Doxyfile.in"
        "${PROJECT_SOURCE_DIR}/Doxyfile"
        @ONLY)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
        "${PROJECT_SOURCE_DIR}/docs/Download.md"
        "${PROJECT_SOURCE_DIR}/Doxyfile")

    add_custom_target(docs
        "${DOXYGEN_EXECUTABLE}" Doxyfile
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

option(PACKAGE_LINUX "Create complete package for Linux" OFF)
option(PACKAGE_WIN32 "Create complete package for Windows" OFF)
option(PACKAGE_OSX "Create complete package for Mac OSX" OFF)
option(PACKAGE_PI2 "Create complete package for Raspberry Pi 2" OFF)

include(ExternalProject)
include(./DownloadCMakeBinaries.cmake)
include(./DownloadSpin2cpp.cmake)

include(./CMakePropWareInstall.cmake)
include(./CMakeCPack.cmake)
