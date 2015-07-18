cmake_minimum_required (VERSION 3.0.0)


set(CMAKE_TOOLCHAIN_FILE ./PropellerToolchain.cmake)
include(./CMakePropellerDefOptions.cmake)
set(CMAKE_BUILD_TYPE None)

project(PropWare
    VERSION 2.0.0
    LANGUAGES C CXX ASM COGC COGCXX ECOGC ECOGCXX DAT)

set(PROPWARE_MAIN_PACKAGE 1)

add_subdirectory(simple)
add_subdirectory(libpropeller)
add_subdirectory(PropWare)
add_subdirectory(Examples)

# TODO: Use find_package(Doxygen) instead of explicitely invoking the doxygen command
add_custom_target(docs
    doxygen Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include(ExternalProject)
set(CUSTOM_LINUX_CMAKE_INSTALL_DIR cmake)
set(CUSTOM_WIN32_CMAKE_INSTALL_DIR CMake)
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
