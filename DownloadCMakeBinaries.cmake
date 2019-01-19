set(CMAKE_DOWNLOAD_VERSION 3.13.3)
set(CUSTOM_LINUX_CMAKE_INSTALL_DIR LinuxCMake)
set(CUSTOM_WIN32_CMAKE_INSTALL_DIR WinCMake)
set(CUSTOM_OSX_CMAKE_INSTALL_DIR   OsxCMake)
set(CUSTOM_PI2_CMAKE_INSTALL_DIR   Pi2CMake)

if (PACKAGE_LINUX)
    externalproject_add(CMake
        PREFIX CMake-src
        URL https://github.com/Kitware/CMake/releases/download/v${CMAKE_DOWNLOAD_VERSION}/cmake-${CMAKE_DOWNLOAD_VERSION}-Linux-x86_64.tar.gz
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR})
endif ()

if (PACKAGE_WIN32)
    externalproject_add(WinCMake
        PREFIX WinCMake-src
        URL https://github.com/Kitware/CMake/releases/download/v${CMAKE_DOWNLOAD_VERSION}/cmake-${CMAKE_DOWNLOAD_VERSION}-win32-x86.zip
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR})
endif ()

if (PACKAGE_OSX)
    externalproject_add(OSXCMake
        PREFIX OSXCMake-src
        URL https://github.com/Kitware/CMake/releases/download/v${CMAKE_DOWNLOAD_VERSION}/cmake-${CMAKE_DOWNLOAD_VERSION}-Darwin-x86_64.tar.gz
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_OSX_CMAKE_INSTALL_DIR})
endif ()

if (PACKAGE_PI2)
    externalproject_add(Pi2CMake
        PREFIX Pi2CMake-src
        URL http://david.zemon.name/downloads/cmake-3.5.20160306-g7cc82-Linux-armv7l.tar.gz
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
        BUILD_COMMAND ${CMAKE_COMMAND} -E echo_append
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${CMAKE_BINARY_DIR}/${CUSTOM_PI2_CMAKE_INSTALL_DIR})
endif ()
