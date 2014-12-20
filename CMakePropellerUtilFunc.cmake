include(${PROPWARE_PATH}/CMakePropellerLibDeps.cmake)
include(${PROPWARE_PATH}/CMakePropellerSetLinker.cmake)

macro (createTopProject projectName)

    include(${PROPWARE_PATH}/CMakePropellerSetFlags.cmake)
    setLibraryDependencies(${projectName})
    setLinker(${projectName})

    # Only add these custom targets if we're not compiling the PropWare library
    if (NOT DEFINED PROPWARE_MAIN_PACKAGE)
        if (DEFINED BOARD)
            set(BOARDFLAG -b${BOARD})
        endif()

        # Add target for run (load to RAM and start terminal)
        add_custom_target(debug
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -t
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

        # Add target for run (load to EEPROM, do not start terminal)
        add_custom_target(run
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -e
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})
    endif ()

endmacro()

macro (createProject projectName)

    include(${PROPWARE_PATH}/CMakePropellerSetFlags.cmake)
    setLibraryDependencies(${projectName})
    setLinker(${projectName})

    # Only add these custom targets if we're not compiling the PropWare library
    if (NOT DEFINED PROPWARE_MAIN_PACKAGE)
        if (DEFINED BOARD)
            set(BOARDFLAG -b${BOARD})
        endif()

        # Add target for run (load to RAM and start terminal)
        add_custom_target(debug-${projectName}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -t
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

        # Add target for run (load to EEPROM, do not start terminal)
        add_custom_target(run-${projectName}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -e
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})
    endif ()

endmacro()