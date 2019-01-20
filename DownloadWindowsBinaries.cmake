if (PACKAGE_WIN32)
    set(CMAKE_DOWNLOAD_VERSION 3.13.3)

    # These variables are needed at install time
    set(CMAKE_BINARY_SHORT_VERSION 3.13)
    set(CMAKE_UNZIP_DIR "${CMAKE_BINARY_DIR}/CMakeBinary/unzipped")
    set(CMAKE_BINARY_INSTALLABLE_DIRECTORY "${CMAKE_UNZIP_DIR}/cmake-${CMAKE_DOWNLOAD_VERSION}-win64-x64/")

    set(CMAKE_BIN_FILE "${CMAKE_BINARY_DIR}/CMakeBinary/zipped/cmake-${CMAKE_DOWNLOAD_VERSION}-win64-x64.zip")
    if (NOT EXISTS "${CMAKE_BIN_FILE}")
        file(DOWNLOAD https://github.com/Kitware/CMake/releases/download/v${CMAKE_DOWNLOAD_VERSION}/cmake-${CMAKE_DOWNLOAD_VERSION}-win64-x64.zip
            "${CMAKE_BIN_FILE}"
            SHOW_PROGRESS
        )
        execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_UNZIP_DIR}")
    endif ()

    add_custom_target(win_cmake ALL DEPENDS "${CMAKE_BINARY_INSTALLABLE_DIRECTORY}/bin/cmake.exe")
    add_custom_command(OUTPUT "${CMAKE_UNZIP_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_UNZIP_DIR}"
    )
    add_custom_command(OUTPUT "${CMAKE_BINARY_INSTALLABLE_DIRECTORY}/bin/cmake.exe"
        COMMAND "${CMAKE_COMMAND}" -E tar -xf "${CMAKE_BIN_FILE}"
        DEPENDS "${CMAKE_BIN_FILE}" "${CMAKE_UNZIP_DIR}"
        WORKING_DIRECTORY "${CMAKE_UNZIP_DIR}"
    )

    set(DOWNLOAD_SPIN2CPP_PREFIX http://david.zemon.name:8111/repository/download/Spin2Cpp_)
    set(DOWNLOAD_SPIN2CPP_WIN_BINARY "${CMAKE_BINARY_DIR}/Spin2Cpp/spin2cpp.exe")
    file(DOWNLOAD ${DOWNLOAD_SPIN2CPP_PREFIX}Windows/:lastSuccessful/spin2cpp.zip!/spin2cpp.exe
        "${DOWNLOAD_SPIN2CPP_WIN_BINARY}"
        SHOW_PROGRESS
    )
endif ()
