cmake_minimum_required (VERSION 3.0.0)

set(PROPWARE_MAIN_PACKAGE 1)

file(READ ${CMAKE_CURRENT_LIST_DIR}/version.txt VERSION)
string(STRIP ${VERSION} VERSION)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
find_package(PropWare ${VERSION} REQUIRED)

# Do some initial configuration that would normally be handled via FindPropWare.cmake
set(CMAKE_BUILD_TYPE None)

# Set the project, version number and all languages used
project(PropWare
    VERSION ${VERSION}
    LANGUAGES C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT)

add_subdirectory(simple)
add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif ()

include(ExternalProject)
set(CMAKE_MAJOR_VERSION 3)
set(CMAKE_MINOR_VERSION 2)
set(CMAKE_PATCH_VERSION 3)
set(CUSTOM_LINUX_CMAKE_INSTALL_DIR pwcmake)
set(CUSTOM_WIN32_CMAKE_INSTALL_DIR PWCMake)
set(CUSTOM_OSX_CMAKE_INSTALL_DIR   OSXCMake)
ExternalProject_Add(CMake
    PREFIX CMake-src
    URL http://www.cmake.org/files/v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}-Linux-x86_64.tar.gz
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR})
ExternalProject_Add(WinCMake
    PREFIX WinCMake-src
    URL http://www.cmake.org/files/v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}-win32-x86.zip
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR})
ExternalProject_Add(OSXCMake
    PREFIX OSXCMake-src
    URL http://www.cmake.org/files/v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}-Darwin-x86_64.tar.gz
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_OSX_CMAKE_INSTALL_DIR})

include(./CMakePropWareInstall.cmake)
include(./CPackConfig.cmake)
include(CPack)
