#.rst:
# FindPropWare
# ------------
#
# Find PropWare
#
# This module finds the required libraries and headers for the PropWare, Simple and libpropeller
# HALs for the Parallax Propeller. Configuration files that aid in Parallax Propeller development
# (including a toolchain file) will also be loaded.
#
# This module sets the following result variables:
#
# ::
#
#   PropWare_FOUND
#   PropWare_VERSION
#   PropWare_INCLUDE_DIR
#   PropWare_<Memory model>_LIBRARIES       A variable is created which includes libraries for all
#                                           of the HALs for a given memory model.
#                                           Ex: PropWare_LMM_LIBRARIES
#   PropWare_<HAL>_<Memory model>_LIBRARY   A variable is created for each HAL and for each memory
#                                           model. Ex: PropWare_PropWare_LMM_LIBRARY or
#                                           PropWare_librpropeller_XMMC_LIBRARY
#
# Example Usages:
#
# ::
#
#   find_package(PropWare REQUIRED)
#   project(Hello)
#   create_simple_executable(${PROJECT_NAME} main.c)
#
#==============================================================================
# The MIT License (MIT)
#
# Copyright (c) 2013 David Zemon
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#==============================================================================

if (PropWare_FOUND STREQUAL "PropWare-NOTFOUND" OR NOT DEFINED PropWare_FOUND)
    ###############################
    # Compile options
    ###############################
    # Compilation options
    option(32_BIT_DOUBLES "Set all doubles to 32-bits (-m32bit-doubles)" ON)
    option(WARN_ALL "Turn on all compiler warnings (-Wall)" ON)
    option(AUTO_C_STD "Set C standard to c99 (-std=c99)" ON)
    option(AUTO_CXX_STD "Set C++ standard to gnu++0x (-std=gnu++0x)" ON)

    # Optimize size option
    option(AUTO_CUT_SECTIONS "Cut out unused code (Compile: -ffunction-sections -fdata-sections; Link: --gc-sections)" ON)

    ###############################
    # Libraries to link
    ###############################

    # TODO: Use "COMPONENT"s to enable/disable certain libraries
    # Libraries to componentize:
    #   libpropeller
    #   Simple
    #   PropWare
    #   tiny
    #   math

    if (DEFINED PROPWARE_MAIN_PACKAGE)
        set(PROPWARE_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
        set(CMAKE_TOOLCHAIN_FILE                        "${PROPWARE_PATH}/CMakeModules/PropellerToolchain.cmake")
        set(PropWare_INCLUDE_DIR                        "${PROPWARE_PATH} ${PROPWARE_PATH}/simple")
        set(PropWare_PropWare_COG_LIBRARY               PropWare_cog)
        set(PropWare_PropWare_CMM_LIBRARY               PropWare_cmm)
        set(PropWare_PropWare_LMM_LIBRARY               PropWare_lmm)
        set(PropWare_PropWare_XMMC_LIBRARY              PropWare_xmmc)
        set(PropWare_PropWare_XMM-SPLIT_LIBRARY         PropWare_xmm-split)
        set(PropWare_PropWare_XMM-SINGLE_LIBRARY        PropWare_xmm-single)
        set(PropWare_libpropeller_COG_LIBRARY           Libpropeller_cog)
        set(PropWare_libpropeller_CMM_LIBRARY           Libpropeller_cmm)
        set(PropWare_libpropeller_LMM_LIBRARY           Libpropeller_lmm)
        set(PropWare_libpropeller_XMMC_LIBRARY          Libpropeller_xmmc)
        set(PropWare_libpropeller_XMM-SPLIT_LIBRARY     Libpropeller_xmm-split)
        set(PropWare_libpropeller_XMM-SINGLE_LIBRARY    Libpropeller_xmm-single)
        set(PropWare_Simple_COG_LIBRARY                 Simple_cog)
        set(PropWare_Simple_CMM_LIBRARY                 Simple_cmm)
        set(PropWare_Simple_LMM_LIBRARY                 Simple_lmm)
        set(PropWare_Simple_XMMC_LIBRARY                Simple_xmmc)
        set(PropWare_Simple_XMM-SPLIT_LIBRARY           Simple_xmm-split)
        set(PropWare_Simple_XMM-SINGLE_LIBRARY          Simple_xmm-single)
    else ()
        if (NOT DEFINED PROPWARE_PATH
            OR NOT EXISTS "${PROPWARE_PATH}/include/PropWare/PropWare.h"
            OR NOT EXISTS "${PROPWARE_PATH}/CMakePropWareInstall.cmake")

            find_path(PROPWARE_PATH
                NAMES
                    ./CMakePropWareInstall.cmake  # We're either looking for PropWare's root source folder
                    include/PropWare/PropWare.h   # ... or we're looking for the key PropWare header in a system folder
                PATHS
                    # If we're working inside the PropWare project...
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../.. # This one used for the Examples directory and PropWare/tests

                    # Or outside the PropWare project
                    $ENV{PROPWARE_PATH} # Check the environment first
                    ${CMAKE_ROOT}/../../../PropWare # Or finally, go with the installed version next to pwcmake
            )
        endif ()

        find_file(CMAKE_TOOLCHAIN_FILE PropellerToolchain.cmake
            PATHS
                ${PROPWARE_PATH}/CMakeModules
                ${CMAKE_ROOT}/Modules)

        # Include directory
        if (EXISTS "${PROPWARE_PATH}/include/PropWare/PropWare.h")
            set(PropWare_INCLUDE_DIR ${PROPWARE_PATH}/include)
        else ()
            set(PropWare_INCLUDE_DIR ${PROPWARE_PATH})
        endif ()

        # Libraries
        if (NOT DEFINED CMAKE_FIND_LIBRARY_PREFIXES)
            set(CMAKE_FIND_LIBRARY_PREFIXES lib)
        endif ()
        if (NOT DEFINED CMAKE_FIND_LIBRARY_SUFFIXES)
            set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
        endif ()

        foreach(memoryModel cog cmm lmm xmmc xmm-single xmm-split)
            string(TOUPPER ${memoryModel} UPPER_MEM_MODEL)

            find_library(PropWare_PropWare_${UPPER_MEM_MODEL}_LIBRARY  PropWare_${memoryModel}
                PATHS
                    ${PROPWARE_PATH}/lib
                    ${PROPWARE_PATH}/bin/PropWare/${memoryModel}
                    ${PROPWARE_PATH}/PropWare/${memoryModel})
            find_library(PropWare_libpropeller_${UPPER_MEM_MODEL}_LIBRARY  Libpropeller_${memoryModel}
                PATHS
                    ${PROPWARE_PATH}/lib
                    ${PROPWARE_PATH}/bin/libpropeller/source/${memoryModel}
                    ${PROPWARE_PATH}/libpropeller/source/${memoryModel})
            find_library(PropWare_Simple_${UPPER_MEM_MODEL}_LIBRARY  Simple_${memoryModel}
                PATHS
                    ${PROPWARE_PATH}/lib
                    ${PROPWARE_PATH}/bin/simple/${memoryModel}
                    ${PROPWARE_PATH}/simple/${memoryModel})
        endforeach()
    endif ()

    if (NOT "${PROPWARE_PATH}" STREQUAL "PROPWARE_PATH-NOTFOUND")
        find_file(PropWare_DAT_SYMBOL_CONVERTER datSymbolConverter.py
            PATHS ${PROPWARE_PATH}/util)
        find_file(PROPWARE_RUN_OBJCOPY CMakeRunObjcopy.cmake
            PATHS
                ${PROPWARE_PATH}/CMakeModules
                ${CMAKE_ROOT}/Modules)

        set(PropWare_LIBRARIES
            # Built-ins
            m

            # HALs
            ${PropWare_PropWare_CMM_LIBRARY}
            ${PropWare_PropWare_LMM_LIBRARY}
            ${PropWare_PropWare_XMMC_LIBRARY}
            ${PropWare_PropWare_XMM-SPLIT_LIBRARY}
            ${PropWare_PropWare_XMM-SINGLE_LIBRARY}
            ${PropWare_libpropeller_COG_LIBRARY}
            ${PropWare_libpropeller_CMM_LIBRARY}
            ${PropWare_libpropeller_LMM_LIBRARY}
            ${PropWare_libpropeller_XMMC_LIBRARY}
            ${PropWare_libpropeller_XMM-SPLIT_LIBRARY}
            ${PropWare_libpropeller_XMM-SINGLE_LIBRARY}
            ${PropWare_Simple_COG_LIBRARY}
            ${PropWare_Simple_CMM_LIBRARY}
            ${PropWare_Simple_LMM_LIBRARY}
            ${PropWare_Simple_XMMC_LIBRARY}
            ${PropWare_Simple_XMM-SPLIT_LIBRARY}
            ${PropWare_Simple_XMM-SINGLE_LIBRARY})

        set(PropWare_COG_LIBRARIES
            ${PropWare_PropWare_COG_LIBRARY}
            ${PropWare_libpropeller_COG_LIBRARY}
            ${PropWare_Simple_COG_LIBRARY})
        set(PropWare_CMM_LIBRARIES
            ${PropWare_PropWare_CMM_LIBRARY}
            ${PropWare_libpropeller_CMM_LIBRARY}
            ${PropWare_Simple_CMM_LIBRARY})
        set(PropWare_LMM_LIBRARIES
            ${PropWare_PropWare_LMM_LIBRARY}
            ${PropWare_libpropeller_LMM_LIBRARY}
            ${PropWare_Simple_LMM_LIBRARY})
        set(PropWare_XMMC_LIBRARIES
            ${PropWare_PropWare_XMMC_LIBRARY}
            ${PropWare_libpropeller_XMMC_LIBRARY}
            ${PropWare_Simple_XMMC_LIBRARY})
        set(PropWare_XMM-SINGLE_LIBRARIES
            ${PropWare_PropWare_XMM-SINGLE_LIBRARY}
            ${PropWare_libpropeller_XMM-SINGLE_LIBRARY}
            ${PropWare_Simple_XMM-SINGLE_LIBRARY})
        set(PropWare_XMM-SPLIT_LIBRARIES
            ${PropWare_PropWare_XMM-SPLIT_LIBRARY}
            ${PropWare_libpropeller_XMM-SPLIT_LIBRARY}
            ${PropWare_Simple_XMM-SPLIT_LIBRARY})

        file(READ ${PROPWARE_PATH}/version.txt PropWare_VERSION)
        string(STRIP ${PropWare_VERSION} PropWare_VERSION)

        ##########################################
        # PropWare helper functions & macros
        ##########################################
        function (set_linker executable)

            macro (list_contains result request)
              set (${result})
              foreach (listItem ${ARGN})
                if (${request} STREQUAL ${listItem})
                  set (${result} TRUE)
                endif ()
              endforeach ()
            endmacro()

            # Set the correct linker language
            get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
            list_contains(use_c C ${_languages})
            if (use_c)
                set(linker_language C)
            else ()
                list_contains(use_cxx CXX ${_languages})
                if (use_cxx)
                    set(linker_language CXX)
                else ()
                    message(FATAL_ERROR
                        "PropWare requires at linking with C or CXX. Please enable at least one of those languages")
                endif ()
            endif ()

            SET_TARGET_PROPERTIES(${executable}
                PROPERTIES
                LINKER_LANGUAGE
                ${linker_language})

        endfunction ()

        macro (set_compile_flags)
            if (NOT DEFINED MODEL)
                message(FATAL_ERROR "MODEL is not defined. You must define MODEL as one of cog, cmm, lmm, xmmc, xmm-single or xmm-split")
            endif ()

            include_directories(${PropWare_INCLUDE_DIR})

            # Handle user options
            if (32_BIT_DOUBLES)
                if (${32_BIT_DOUBLES_SET})
                else ()
                    set(32_BIT_DOUBLES_SET 1)
                    set(C_FLAGS "${C_FLAGS} -m32bit-doubles")
                    set(CXX_FLAGS "${CXX_FLAGS} -m32bit-doubles")
                endif ()
            endif ()

            if (WARN_ALL)
                if (${WARN_ALL_SET})
                else ()
                    set(WARN_ALL_SET 1)
                    set(C_FLAGS "${C_FLAGS} -Wall")
                    set(CXX_FLAGS "${CXX_FLAGS} -Wall")
                endif ()
            endif ()

            if (AUTO_C_STD)
                if (${AUTO_C_STD_SET})
                else ()
                    set(AUTO_C_STD_SET 1)
                    set(C_FLAGS "${C_FLAGS} -std=c99")
                endif ()
            endif ()

            if (AUTO_CXX_STD)
                if (${AUTO_CXX_STD_SET})
                else ()
                    set(AUTO_CXX_STD_SET 1)
                    set(CXX_FLAGS "${CXX_FLAGS} -std=gnu++0x")
                endif ()
            endif ()

            # Linker pruning is broken when used with the cog memory model. See the
            # following thread for a workaround:
            # http://forums.parallax.com/showthread.php/157878-Simple-blinky-program-and-linker-pruning
            string(TOLOWER ${MODEL} MODEL_LOWERCASE)
            if (NOT((MODEL_LOWERCASE STREQUAL "cog")))
                if (AUTO_CUT_SECTIONS)
                    if (${AUTO_CUT_SECTIONS_SET})
                    else ()
                        set(AUTO_CUT_SECTIONS_SET 1)
                        set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
                        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
                    endif ()
                endif ()
            endif ()

            # Check if a deprecated variable name is set
            if (DEFINED CFLAGS OR DEFINED CXXFLAGS)
                if (${CFLAGS_CXXFLAGS_SET})
                else ()
                    set(CFLAGS_CXXFLAGS_SET 1)
                    message(WARN ": The variables `CFLAGS` and `CXXFLAGS` have been replaced by `C_FLAGS` and `CXX_FLAGS`.")
                    set(C_FLAGS ${CFLAGS})
                    set(CXX_FLAGS ${CXXFLAGS})
                    set(CFLAGS )
                    set(CXXFLAGS )
                endif ()
            endif()

            if (${COMMON_FLAGS_SET})
            else ()
                set(COMMON_FLAGS_SET 1)
                set(COMMON_FLAGS "-save-temps ${COMMON_FLAGS}")
                #set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=main.rawmap ${CMAKE_EXE_LINKER_FLAGS}")
            endif ()

            # Overwite the old flags
            set(CMAKE_ASM_FLAGS     "${COMMON_FLAGS}    ${ASM_FLAGS}")
            set(CMAKE_C_FLAGS       "${COMMON_FLAGS}    ${C_FLAGS}")
            set(CMAKE_CXX_FLAGS     "${COMMON_FLAGS}    ${CXX_FLAGS}")

            set(CMAKE_COGC_FLAGS    "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${COGC_FLAGS}")
            set(CMAKE_COGCXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${COGCXX_FLAGS}")
            set(CMAKE_ECOGC_FLAGS   "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${ECOGC_FLAGS}")
            set(CMAKE_ECOGCXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${ECOGCXX_FLAGS}")

            # If no model is specified, we must choose a default so that the proper libraries can be linked
            if (NOT DEFINED MODEL)
                set(MODEL lmm)
            endif ()

            # XMM model is retroactively renamed xmm-split
            if (${MODEL} STREQUAL xmm)
                set(MODEL xmm-split)
            endif ()

            set(CMAKE_ASM_FLAGS     "${CMAKE_ASM_FLAGS} -m${MODEL}")
            set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}   -m${MODEL}")
            set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -m${MODEL}")
            set(CMAKE_DAT_FLAGS     "${MODEL}")

        endmacro()

        macro (add_prop_targets name)

            # Add target for debugging (load to RAM and start GDB)
            add_custom_target(gdb
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -g &&
                    ${CMAKE_GDB} ${BAUDFLAG} $<TARGET_FILE:${name}>
                    DEPENDS ${name})

            # Add target for run (load to RAM and start terminal)
            add_custom_target(debug
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -t
                    DEPENDS ${name})

            # Add target for run (load to EEPROM, do not start terminal)
            add_custom_target(run-ram
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r
                    DEPENDS ${name})

            # Add target for run (load to EEPROM, do not start terminal)
            add_custom_target(run
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -e
                    DEPENDS ${name})

        endmacro()

        macro (add_prop_targets_with_name name)

            # Add target for debugging (load to RAM and start GDB)
            add_custom_target(gdb-${name}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -g &&
                    ${CMAKE_GDB} ${BAUDFLAG} $<TARGET_FILE:${name}>
                    DEPENDS ${name})

            # Add target for run (load to RAM and start terminal)
            add_custom_target(debug-${name}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -t
                    DEPENDS ${name})

            # Add target for run (load to EEPROM, do not start terminal)
            add_custom_target(run-ram-${name}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r
                    DEPENDS ${name})

            # Add target for run (load to EEPROM, do not start terminal)
            add_custom_target(run-${name}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r
                    DEPENDS ${name})

        endmacro()

        macro(set_propware_flags projectName)
            set_compile_flags()
            string(TOUPPER ${MODEL} UPPER_MODEL)
            target_link_libraries(${projectName} ${PropWare_${UPPER_MODEL}_LIBRARIES})
            set_linker(${projectName})

            if (DEFINED BOARD)
                set(BOARDFLAG -b${BOARD})
            endif()

            if (DEFINED GDB_BAUD)
                set(BAUDFLAG -b ${GDB_BAUD})
            elseif (DEFINED ENV{GDB_BAUD})
                set(BAUDFLAG -b $ENV{GDB_BAUD})
            endif ()
        endmacro()

        macro (create_executable name src1)
            add_executable(${name} ${src1} ${ARGN})
            set_propware_flags(${name})
            add_prop_targets_with_name(${name})
        endmacro()

        macro (create_simple_executable name src1)
            add_executable(${name} ${src1} ${ARGN})
            set_propware_flags(${name})

            if (NOT PROJECT_NAME STREQUAL "PropWare" AND DEFINED PROPWARE_MAIN_PACKAGE)
                add_prop_targets_with_name(${name})
            else ()
                add_prop_targets(${name})
            endif ()
        endmacro()
    endif ()

    enable_testing()
    add_custom_target(test-all COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
    macro(create_test target src1)
        create_executable(${target} ${src1} ${ARGN})
        add_test(NAME ${target}
            COMMAND ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${target}> -r -t -q)
        add_dependencies(test-all ${target})
        add_custom_target(test-${target}
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ${target}
            DEPENDS ${target})
    endmacro()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(PropWare
        FOUND_VAR PropWare_FOUND
        REQUIRED_VARS
            PROPWARE_PATH
            PropWare_INCLUDE_DIR
            PropWare_PropWare_COG_LIBRARY
            PropWare_PropWare_CMM_LIBRARY
            PropWare_PropWare_LMM_LIBRARY
            PropWare_PropWare_XMMC_LIBRARY
            PropWare_PropWare_XMM-SPLIT_LIBRARY
            PropWare_PropWare_XMM-SINGLE_LIBRARY
            PropWare_libpropeller_COG_LIBRARY
            PropWare_libpropeller_CMM_LIBRARY
            PropWare_libpropeller_LMM_LIBRARY
            PropWare_libpropeller_XMMC_LIBRARY
            PropWare_libpropeller_XMM-SPLIT_LIBRARY
            PropWare_libpropeller_XMM-SINGLE_LIBRARY
            PropWare_Simple_COG_LIBRARY
            PropWare_Simple_CMM_LIBRARY
            PropWare_Simple_LMM_LIBRARY
            PropWare_Simple_XMMC_LIBRARY
            PropWare_Simple_XMM-SPLIT_LIBRARY
            PropWare_Simple_XMM-SINGLE_LIBRARY
        VERSION_VAR PropWare_VERSION
    )

    mark_as_advanced(
        CMAKE_TOOLCHAIN_FILE
        PropWare_VERSION

        PropWare_INCLUDE_DIR

        PropWare_LIBRARIES
        PropWare_COG_LIBRARIES
        PropWare_CMM_LIBRARIES
        PropWare_LMM_LIBRARIES
        PropWare_XMMC_LIBRARIES
        PropWare_XMM-SPLIT_LIBRARIES
        PropWare_XMM-SINGLE_LIBRARIES
        PropWare_PropWare_COG_LIBRARY
        PropWare_PropWare_CMM_LIBRARY
        PropWare_PropWare_LMM_LIBRARY
        PropWare_PropWare_XMMC_LIBRARY
        PropWare_PropWare_XMM-SPLIT_LIBRARY
        PropWare_PropWare_XMM-SINGLE_LIBRARY
        PropWare_libpropeller_COG_LIBRARY
        PropWare_libpropeller_CMM_LIBRARY
        PropWare_libpropeller_LMM_LIBRARY
        PropWare_libpropeller_XMMC_LIBRARY
        PropWare_libpropeller_XMM-SPLIT_LIBRARY
        PropWare_libpropeller_XMM-SINGLE_LIBRARY
        PropWare_Simple_COG_LIBRARY
        PropWare_Simple_CMM_LIBRARY
        PropWare_Simple_LMM_LIBRARY
        PropWare_Simple_XMMC_LIBRARY
        PropWare_Simple_XMM-SPLIT_LIBRARY
        PropWare_Simple_XMM-SINGLE_LIBRARY
    )
endif()
