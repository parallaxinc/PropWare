install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_LINUX_CMAKE_INSTALL_DIR}
    DESTINATION .
    USE_SOURCE_PERMISSIONS
    COMPONENT cmake)
if (NOT WIN32)
    macro(InstallSymlink _filepath _sympath)
        get_filename_component(_symname ${_sympath} NAME)

        # Don't use get_filename_component because it corrupts escape characters
        string(LENGTH ${_symname} _namelen)
        string(LENGTH ${_sympath} _totalpathlen)
        math(EXPR _pathlen "${_totalpathlen} - ${_namelen}")
        string(SUBSTRING ${_sympath} 0 ${_pathlen} _installdir)

        if (BINARY_PACKAGING_MODE)
            execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink
                            ${_filepath}
                            ${CMAKE_CURRENT_BINARY_DIR}/${_symname})
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_symname}
                    DESTINATION ${_installdir}
                    ${ARGN})
        else ()
            # scripting the symlink installation at install time should work
            # for CMake 2.6.x and 2.8.x
            install(CODE "
                    if (\"\$ENV{DESTDIR}\" STREQUAL \"\")
                        set(_realInstallDir ${_installdir})
                    else ()
                        set(_realInstallDir \$ENV{DESTDIR}/${_installdir})
                    endif ()
                    execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E make_directory \"\${_realInstallDir}\")
                    execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E create_symlink
                                    ${_filepath}
                                    \${_realInstallDir}/${_symname})"
                ${ARGN})
        endif ()
    endmacro(InstallSymlink)


    InstallSymlink(../${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/bin/cmake
        \\\${CMAKE_INSTALL_PREFIX}/bin/cmake
        COMPONENT cmake)
    InstallSymlink(../${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/bin/ccmake
        \\\${CMAKE_INSTALL_PREFIX}/bin/ccmake
        COMPONENT cmake)
    InstallSymlink(../${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/bin/cmake-gui
        \\\${CMAKE_INSTALL_PREFIX}/bin/cmake-gui
        COMPONENT cmake)
    InstallSymlink(../${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/bin/cpack
        \\\${CMAKE_INSTALL_PREFIX}/bin/cpack
        COMPONENT cmake)
    InstallSymlink(../${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/bin/ctest
        \\\${CMAKE_INSTALL_PREFIX}/bin/ctest
        COMPONENT cmake)
endif ()
install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_WIN32_CMAKE_INSTALL_DIR}
    DESTINATION .
    USE_SOURCE_PERMISSIONS
    COMPONENT win_cmake)
install(DIRECTORY ${CMAKE_BINARY_DIR}/${CUSTOM_OSX_CMAKE_INSTALL_DIR}/CMake.app/
    DESTINATION pwcmake.app
    USE_SOURCE_PERMISSIONS
    COMPONENT osx_cmake)

install(DIRECTORY CMakeModules/
    DESTINATION Modules
    COMPONENT standalone-cmake)
install(DIRECTORY CMakeModules/
    DESTINATION ${CUSTOM_LINUX_CMAKE_INSTALL_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
    COMPONENT cmake)
install(DIRECTORY CMakeModules/
    DESTINATION ${CUSTOM_WIN32_CMAKE_INSTALL_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
    COMPONENT win_cmake)
install(DIRECTORY CMakeModules/
    DESTINATION pwcmake.app/Contents/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules
    COMPONENT osx_cmake)

# Spin2cpp
install(FILES ${CMAKE_BINARY_DIR}/Spin2Cpp/src/Spin2Cpp/build/spin2cpp
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    COMPONENT linux_spin2cpp)
install(FILES ${CMAKE_BINARY_DIR}/Spin2Cpp/src/Spin2Cpp/build-win32/spin2cpp.exe
    DESTINATION .
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    COMPONENT win_spin2cpp)

# PropWare & libpropeller includes
install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/PropWare
        ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller
    DESTINATION PropWare/include
    COMPONENT propware
    FILES_MATCHING PATTERN *.h
    PATTERN libpropeller/libpropeller/compile_tools EXCLUDE
    PATTERN libpropeller/libpropeller/unity_tools/asmsrc EXCLUDE)

# Version file
file(WRITE "${PROJECT_BINARY_DIR}/version.txt" "${PROJECT_VERSION}")
install(FILES
        "${PROJECT_BINARY_DIR}/version.txt"
    DESTINATION PropWare
    COMPONENT propware)

# Examples
install(DIRECTORY Examples
    DESTINATION PropWare
    COMPONENT examples
    PATTERN bin EXCLUDE
    PATTERN Examples/CMakeLists.txt EXCLUDE
    PATTERN *~ EXCLUDE
    PATTERN .idea EXCLUDE)

install(EXPORT PropWare-targets
    COMPONENT propware
    DESTINATION PropWare/lib)
