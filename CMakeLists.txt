cmake_minimum_required (VERSION 3.0.0)

set(PROPWARE_MAIN_PACKAGE TRUE)

# Get the PropWare version
file(READ ${CMAKE_CURRENT_LIST_DIR}/version.txt VERSION)
string(STRIP ${VERSION} VERSION)

# Load special modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
find_package(PropWare ${VERSION} REQUIRED)

# Allow using `if (VAR IN_LIST MY_LIST)`
cmake_policy(SET CMP0057 NEW)

# Set the project, version number and all languages used
project(PropWare
    VERSION ${VERSION}
    LANGUAGES C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT SPIN2DAT)

add_subdirectory(simple)
add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)
add_subdirectory(test)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif ()

include(ExternalProject)
include(./DownloadCMakeBinaries.cmake)
include(./DownloadSpin2cpp.cmake)

include(./CMakePropWareInstall.cmake)
include(./CMakeCPack.cmake)
