if (PACKAGE_LINUX)
    install(DIRECTORY CMakeModules/
        DESTINATION share/PropWare/CMakeModules
        COMPONENT linux_cmakemodules)
endif ()

if (PACKAGE_WIN32)
    install(DIRECTORY CMakeModules/
        DESTINATION share/cmake-${CMAKE_BINARY_SHORT_VERSION}/Modules
        COMPONENT win_cmake)
    install(DIRECTORY "${CMAKE_BINARY_INSTALLABLE_DIRECTORY}/"
        DESTINATION .
        COMPONENT win_cmake)
    install(PROGRAMS "${DOWNLOAD_SPIN2CPP_WIN_BINARY}"
        DESTINATION bin
        COMPONENT win_spin2cpp)
endif ()

if (PACKAGE_OSX)
    install(DIRECTORY CMakeModules/
        DESTINATION pwcmake.app/Contents/CMakeModules
        COMPONENT macosx_cmakemodules)
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
