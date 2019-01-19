set(DOWNLOAD_PREFIX http://david.zemon.name:8111/repository/download/Spin2Cpp_)

if (PACKAGE_LINUX AND NOT EXISTS "${CMAKE_BINARY_DIR}/Spin2Cpp/Linux/spin2cpp")
    file(DOWNLOAD ${DOWNLOAD_PREFIX}Linux/:lastSuccessful/spin2cpp.tar.gz!/spin2cpp
        "${CMAKE_BINARY_DIR}/Spin2Cpp/Linux/spin2cpp"
        SHOW_PROGRESS
    )
endif ()

if (PACKAGE_WIN32)
    file(DOWNLOAD ${DOWNLOAD_PREFIX}Windows/:lastSuccessful/spin2cpp.zip!/spin2cpp.exe
        "${CMAKE_BINARY_DIR}/Spin2Cpp/Windows/spin2cpp.exe"
        SHOW_PROGRESS
    )
endif ()

if (PACKAGE_RPI)
    file(DOWNLOAD ${DOWNLOAD_PREFIX}RaspberryPi/:lastSuccessful/spin2cpp.tar.gz!/spin2cpp
        "${CMAKE_BINARY_DIR}/Spin2Cpp/Rpi/spin2cpp"
        SHOW_PROGRESS
    )
endif ()

if (PACKAGE_OSX)
    file(DOWNLOAD ${DOWNLOAD_PREFIX}MacOS/:lastSuccessful/spin2cpp.tar.gz!/spin2cpp
        "${CMAKE_BINARY_DIR}/Spin2Cpp/Macosx/spin2cpp"
        SHOW_PROGRESS
    )
endif ()
