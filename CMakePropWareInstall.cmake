install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR}
    DESTINATION PropWare
    COMPONENT cmake)
install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR}
    DESTINATION PropWare
    COMPONENT win_cmake)

install(DIRECTORY CMakeModules/
    DESTINATION PropWare/${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/share/cmake-3.2/Modules
    COMPONENT cmake)
install(DIRECTORY CMakeModules/
    DESTINATION PropWare/${CUSTOM_WIN32_CMAKE_INSTALL_DIR}/share/cmake-3.2/Modules
    COMPONENT win_cmake)

install(FILES
        CMakePropellerDefOptions.cmake
        CMakePropellerHeader.cmake
        CMakePropellerLibDeps.cmake
        CMakePropellerSetFlags.cmake
        CMakePropellerSetLinker.cmake
        CMakePropellerUtilFunc.cmake
    DESTINATION PropWare/${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/share/cmake-3.2/Modules
    COMPONENT cmake)
install(FILES
        CMakePropellerDefOptions.cmake
        CMakePropellerHeader.cmake
        CMakePropellerLibDeps.cmake
        CMakePropellerSetFlags.cmake
        CMakePropellerSetLinker.cmake
        CMakePropellerUtilFunc.cmake
    DESTINATION PropWare/${CUSTOM_WIN32_CMAKE_INSTALL_DIR}/share/cmake-3.2/Modules
    COMPONENT win_cmake)

install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/PropWare
        ${PROJECT_SOURCE_DIR}/simple
        ${PROJECT_SOURCE_DIR}/libpropeller
    DESTINATION PropWare/include
    COMPONENT propware
    FILES_MATCHING PATTERN *.h)
