if (PACKAGE_LINUX)
    install(DIRECTORY CMakeModules/
        DESTINATION share/PropWare/CMakeModules
        COMPONENT cmakemodules)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/Linux/spin2cpp
        DESTINATION bin
        COMPONENT linux_spin2cpp)
endif ()

if (PACKAGE_WIN32)
    install(DIRECTORY CMakeModules
        DESTINATION .
        COMPONENT win_cmake)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/Windows/spin2cpp.exe
        DESTINATION bin
        COMPONENT win_spin2cpp)
endif ()

if (PACKAGE_OSX)
    install(DIRECTORY CMakeModules/
        DESTINATION pwcmake.app/Contents/CMakeModules
        COMPONENT cmakemodules)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/Macosx/spin2cpp
        DESTINATION pwcmake.app/Contents/bin
        COMPONENT linux_spin2cpp)
endif ()

if (PACKAGE_PI2)
    install(DIRECTORY CMakeModules/
        DESTINATION share/PropWare/CMakeModules
        COMPONENT cmakemodules)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/Rpi/spin2cpp
        DESTINATION .
        COMPONENT rpi_spin2cpp)
endif ()

if (PACKAGE_OSX)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/Spin2Cpp/Macosx/spin2cpp
        DESTINATION bin
        COMPONENT macosx_spin2cpp)
endif ()

# PropWare & libpropeller includes
install(DIRECTORY
        "${PROJECT_SOURCE_DIR}/PropWare"
        "${PROJECT_SOURCE_DIR}/libpropeller/libpropeller"
        "${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/"
    DESTINATION share/PropWare/include/c++
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
    DESTINATION share/PropWare/lib
    NAMESPACE PropWare::)
