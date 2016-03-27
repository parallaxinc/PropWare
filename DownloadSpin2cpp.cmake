if (PACKAGE_LINUX OR PACKAGE_WIN32 OR PACKAGE_PI2)
    # Build the make line
    set(MAKES )
    if (PACKAGE_LINUX)
        list(APPEND MAKES make)
    endif ()
    if (PACKAGE_WIN32)
        list(APPEND MAKES "make CROSS=win32")
    endif ()
    if (PACKAGE_PI2)
        list(APPEND MAKES "make CROSS=rpi")
    endif ()

    string(REPLACE ";" " && " BUILD_COMMAND "${MAKES}")
    string(REPLACE " " ";" BUILD_COMMAND "${BUILD_COMMAND}")

    externalproject_add(Spin2Cpp
        PREFIX Spin2Cpp
        BINARY_DIR Spin2Cpp/src/Spin2Cpp
        GIT_REPOSITORY https://github.com/totalspectrum/spin2cpp.git
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
        BUILD_COMMAND ${BUILD_COMMAND}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E echo_append
        )
endif ()
