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
set(CUSTOM_LINUX_CMAKE_INSTALL_DIR pwcmake)
set(CUSTOM_WIN32_CMAKE_INSTALL_DIR PWCMake)
ExternalProject_Add(CMake
    PREFIX CMake-src
    URL http://www.cmake.org/files/v3.2/cmake-3.2.3-Linux-x86_64.tar.gz
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR})
ExternalProject_Add(WinCMake
    PREFIX WinCMake-src
    URL http://www.cmake.org/files/v3.2/cmake-3.2.3-win32-x86.zip
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR})

include(./CMakePropWareInstall.cmake)
include(./CPackConfig.cmake)
set(CPACK_COMPONENT_cmake_DISPLAY_NAME     "CMake")
set(CPACK_COMPONENT_cmake_DESCRIPTION      "Complete CMake installation with additional files for easy Propeller development")
set(CPACK_COMPONENT_win_cmake_DISPLAY_NAME "CMake")
set(CPACK_COMPONENT_win_cmake_DESCRIPTION  "Complete CMake installation with additional files for easy Propeller development")
include(CPack)
