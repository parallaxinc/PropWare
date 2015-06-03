include(${PROPWARE_PATH}/CMakePropellerLibDeps.cmake)
include(${PROPWARE_PATH}/CMakePropellerSetLinker.cmake)

set(ALL_DEPENDS
	PropWare_cmm PropWare_lmm PropWare_xmmc PropWare_xmm-split PropWare_xmm-single
	Libpropeller_cog Libpropeller_cmm Libpropeller_lmm Libpropeller_xmmc Libpropeller_xmm-split Libpropeller_xmm-single
	Simple_cog Simple_cmm Simple_lmm Simple_xmmc Simple_xmm-split Simple_xmm-single)

macro (create_top_project projectName)

    include(${PROPWARE_PATH}/CMakePropellerSetFlags.cmake)
    setLibraryDependencies(${projectName})
    setLinker(${projectName})

    # Only add these custom targets if we're not compiling the PropWare library
    if (DEFINED PROPWARE_MAIN_PACKAGE)
        add_dependencies(${projectName} ${ALL_DEPENDS})
    else ()
        if (DEFINED BOARD)
            set(BOARDFLAG -b${BOARD})
        endif ()

        if (DEFINED GDB_BAUD)
            set(BAUDFLAG -b ${GDB_BAUD})
        elseif (DEFINED ENV{GDB_BAUD})
            set(BAUDFLAG -b $ENV{GDB_BAUD})
        endif ()

        # Add target for debugging (load to RAM and start GDB)
        add_custom_target(gdb
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -g &&
                ${CMAKE_GDB} ${BAUDFLAG} ${projectName}.elf
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

        add_custom_target(test
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -t -q
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

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

macro (create_project projectName)

    include(${PROPWARE_PATH}/CMakePropellerSetFlags.cmake)
    setLibraryDependencies(${projectName})
    setLinker(${projectName})

    # Only add these custom targets if we're not compiling the PropWare library
    if (DEFINED PROPWARE_MAIN_PACKAGE)
        add_dependencies(${projectName} ${ALL_DEPENDS})
    else ()
        if (DEFINED BOARD)
            set(BOARDFLAG -b${BOARD})
        endif()

        if (DEFINED GDB_BAUD)
            set(BAUDFLAG -b ${GDB_BAUD})
        elseif (DEFINED ENV{GDB_BAUD})
            set(BAUDFLAG -b $ENV{GDB_BAUD})
        endif ()

        # Add target for debugging (load to RAM and start GDB)
        add_custom_target(gdb-${projectName}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -g &&
                ${CMAKE_GDB} ${BAUDFLAG} ${projectName}.elf
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

        add_custom_target(test-${projectName}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${projectName}.elf -r -t -q
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${projectName})

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

macro (create_executable name src1)
    add_executable(${name} ${src1} ${ARGN})
    create_project(${name})
endmacro()

macro (create_simple_executable name src1)
    add_executable(${name} ${src1} ${ARGN})
    create_top_project(${name})
endmacro()
