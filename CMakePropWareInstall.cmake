if (PACKAGE_LINUX)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        COMPONENT cmake
        PATTERN doc/* EXCLUDE)

    install(DIRECTORY CMakeModules/
        DESTINATION share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
        COMPONENT cmake)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/src/Spin2Cpp/build/spin2cpp
        DESTINATION bin
        COMPONENT linux_spin2cpp)
endif ()

if (PACKAGE_WIN32)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR}/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        COMPONENT win_cmake)
    install(DIRECTORY CMakeModules/
        DESTINATION share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
        COMPONENT win_cmake)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/src/Spin2Cpp/build-win32/spin2cpp.exe
        DESTINATION bin
        COMPONENT win_spin2cpp)
endif ()

if (PACKAGE_OSX)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_OSX_CMAKE_INSTALL_DIR}/CMake.app
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        COMPONENT osx_cmake)
    install(DIRECTORY CMakeModules/
        DESTINATION pwcmake.app/Contents/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
        COMPONENT osx_cmake)
endif ()

if (PACKAGE_PI2)
    set(PI2_CMAKE_VERSION_MAJOR_MINOR "3.5")
    install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_PI2_CMAKE_INSTALL_DIR}/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        COMPONENT pi2_cmake)
    install(DIRECTORY CMakeModules/
        DESTINATION ${CUSTOM_PI2_CMAKE_INSTALL_DIR}/share/cmake-${PI2_CMAKE_VERSION_MAJOR_MINOR}/Modules
        COMPONENT pi2_cmake)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/src/Spin2Cpp/build-rpi/spin2cpp
        DESTINATION .
        COMPONENT rpi_spin2cpp)
endif ()

install(DIRECTORY CMakeModules/
    DESTINATION Modules
    COMPONENT standalone-cmake)

# PropWare & libpropeller includes
install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/PropWare
        ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller
        ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/
    DESTINATION share/PropWare/include
    COMPONENT propware
    FILES_MATCHING PATTERN *.h
    PATTERN libpropeller/libpropeller/compile_tools EXCLUDE
    PATTERN libpropeller/libpropeller/unity_tools/asmsrc EXCLUDE)

# Version file
file(WRITE "${PROJECT_BINARY_DIR}/version.txt" "${PROJECT_VERSION}")
install(FILES
        "${PROJECT_BINARY_DIR}/version.txt"
    DESTINATION share/PropWare
    COMPONENT propware)

# Examples
install(DIRECTORY Examples
    DESTINATION share/PropWare
    COMPONENT examples
    PATTERN bin EXCLUDE
    PATTERN Examples/CMakeLists.txt EXCLUDE
    PATTERN *~ EXCLUDE
    PATTERN .idea EXCLUDE)

install(EXPORT PropWare-targets
    COMPONENT propware
    DESTINATION share/PropWare/lib)
