externalproject_add(Spin2Cpp
    PREFIX Spin2Cpp
    BINARY_DIR Spin2Cpp/src/Spin2Cpp
    GIT_REPOSITORY https://github.com/totalspectrum/spin2cpp.git
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo_append
    BUILD_COMMAND make && make CROSS=win32
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo_append
    )
