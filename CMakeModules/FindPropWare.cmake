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

set(CMAKE_CONFIGURATION_TYPES None
    CACHE TYPE INTERNAL FORCE)

if (NOT PropWare_FOUND)
    ###############################
    # Compile options
    ###############################
    # General options
    option(32_BIT_DOUBLES "Set all doubles to 32-bits (-m32bit-doubles)" ON)
    option(WARN_ALL "Turn on all compiler warnings (-Wall)" ON)
    option(AUTO_C_STD "Set C standard to c99 (-std=c99)" ON)
    option(AUTO_CXX_STD "Set C++ standard to gnu++0x (-std=gnu++0x)" ON)

    # Size optimizations
    option(AUTO_OTPIMIZATION "Set optimization level to \"size\" (-Os)" ON)
    option(AUTO_CUT_SECTIONS "Cut out unused code (Compile: -ffunction-sections -fdata-sections; Link: --gc-sections)" ON)

    # Language features
    option(EXCEPTIONS "Enable exceptions (requires hundreds of kilobytes of RAM) (-fexceptions/-fno-exceptions" OFF)
    option(RUNTIME_TYPE_INFORMATION "Enable runtime type information (-frtti/-fno-rtti)" OFF)
    option(THREADSAFE_STATICS "Enable threadsafe statics (-fthreadsafe-statics/-fno-threadsafe-statics)" OFF)

    ###############################
    # Libraries to link
    ###############################

    if (PROPWARE_MAIN_PACKAGE)
        set(PROPWARE_PATH                               "${CMAKE_CURRENT_LIST_DIR}/..")
        set(CMAKE_TOOLCHAIN_FILE                        "${PROPWARE_PATH}/CMakeModules/PropellerToolchain.cmake")
        set(PropWare_INCLUDE_DIR                        "${PROPWARE_PATH}" "${PROPWARE_PATH}/libpropeller")
        foreach (component PropWare Libpropeller Simple)
            foreach (model cog cmm lmm xmmc xmm-split xmm-single)
                string(TOUPPER ${model} upper_model)
                set(PropWare_${component}_${upper_model}_LIBRARY ${component}_${model})
            endforeach()
        endforeach()
    else ()
        if (NOT DEFINED PROPWARE_PATH
            OR NOT EXISTS "${PROPWARE_PATH}/lib/PropWare-targets.cmake")
            find_path(PROPWARE_PATH
                NAMES
                    ./CMakePropWareInstall.cmake  # We're either looking for PropWare's root source folder
                    include/PropWare/PropWare.h   # ... or we're looking for the key PropWare header in a system folder
                PATHS
                    "$ENV{PROPWARE_PATH}" # Check the environment first
                    "${CMAKE_ROOT}/../../../PropWare" # Or go with the installed version next to pwcmake
            )
        endif ()

        find_file(CMAKE_TOOLCHAIN_FILE PropellerToolchain.cmake
            PATHS "${CMAKE_ROOT}/Modules")
        find_program(CMAKE_MAKE_PROGRAM make
            PATHS "${PROPWARE_PATH}")

        # Include directory
        if (EXISTS "${PROPWARE_PATH}/include/PropWare/PropWare.h")
            set(PropWare_INCLUDE_DIR "${PROPWARE_PATH}/include")
        else ()
            set(PropWare_INCLUDE_DIR "${PROPWARE_PATH}")
        endif ()

        include("${PROPWARE_PATH}/lib/PropWare-targets.cmake")
        if (PropWare_FIND_COMPONENTS)
            # If we're using componentized search, only grab the requested libraries
            foreach (component IN LISTS PropWare_FIND_COMPONENTS)
                foreach (model cog cmm lmm xmmc xmm-split xmm-single)
                    string(TOUPPER ${model} upper_model)
                    if (TARGET ${component}_${model})
                        set(PropWare_${component}_FOUND 1)
                        set(PropWare_${component}_${upper_model}_LIBRARY ${component}_${model})
                    else ()
                        set(PropWare_${component}_FOUND 0)
                        if (PropWare_FIND_REQUIRED_${component})
                            message(FATAL_ERROR "PropWare's ${component} component not available due to missing ${component}_${model}")
                        endif ()
                    endif ()
                endforeach ()
            endforeach ()
        else ()
            # If we're not using componentized search, grab them all
            foreach (component PropWare Libpropeller Simple)
                foreach (model cog cmm lmm xmmc xmm-split xmm-single)
                    string(TOUPPER ${model} upper_model)
                    set(PropWare_${component}_${upper_model}_LIBRARY ${component}_${model})
                endforeach()
            endforeach()
        endif ()
    endif ()

    if (PROPWARE_PATH)
        find_file(PropWare_DAT_SYMBOL_CONVERTER CMakeDatSymbolConverter.cmake
            PATHS
                "${PROPWARE_PATH}/CMakeModules"
                "${CMAKE_ROOT}/Modules")
        find_file(PropWare_SPIN2DAT_SYMBOL_CONVERTER CMakeSpin2DatSymbolConverter.cmake
            PATHS
                "${PROPWARE_PATH}/CMakeModules"
                "${CMAKE_ROOT}/Modules")
        find_file(PROPWARE_RUN_OBJCOPY CMakeRunObjcopy.cmake
            PATHS
                "${PROPWARE_PATH}/CMakeModules"
                "${CMAKE_ROOT}/Modules")
        find_file(ELF_SIZER CMakeElfSizer.cmake
            PATHS
                "${PROPWARE_PATH}/CMakeModules"
                "${CMAKE_ROOT}/Modules")
        find_program(SPIN2CPP_COMMAND spin2cpp
            "${PROPWARE_PATH}")

        set(PropWare_LIBRARIES
            # Built-ins
            m

            # HALs
            ${PropWare_PropWare_CMM_LIBRARY}
            ${PropWare_PropWare_LMM_LIBRARY}
            ${PropWare_PropWare_XMMC_LIBRARY}
            ${PropWare_PropWare_XMM-SPLIT_LIBRARY}
            ${PropWare_PropWare_XMM-SINGLE_LIBRARY}
            ${PropWare_Libpropeller_COG_LIBRARY}
            ${PropWare_Libpropeller_CMM_LIBRARY}
            ${PropWare_Libpropeller_LMM_LIBRARY}
            ${PropWare_Libpropeller_XMMC_LIBRARY}
            ${PropWare_Libpropeller_XMM-SPLIT_LIBRARY}
            ${PropWare_Libpropeller_XMM-SINGLE_LIBRARY}
            ${PropWare_Simple_COG_LIBRARY}
            ${PropWare_Simple_CMM_LIBRARY}
            ${PropWare_Simple_LMM_LIBRARY}
            ${PropWare_Simple_XMMC_LIBRARY}
            ${PropWare_Simple_XMM-SPLIT_LIBRARY}
            ${PropWare_Simple_XMM-SINGLE_LIBRARY})

        set(PropWare_COG_LIBRARIES
            ${PropWare_PropWare_COG_LIBRARY}
            ${PropWare_Libpropeller_COG_LIBRARY}
            ${PropWare_Simple_COG_LIBRARY})
        set(PropWare_CMM_LIBRARIES
            ${PropWare_PropWare_CMM_LIBRARY}
            ${PropWare_Libpropeller_CMM_LIBRARY}
            ${PropWare_Simple_CMM_LIBRARY})
        set(PropWare_LMM_LIBRARIES
            ${PropWare_PropWare_LMM_LIBRARY}
            ${PropWare_Libpropeller_LMM_LIBRARY}
            ${PropWare_Simple_LMM_LIBRARY})
        set(PropWare_XMMC_LIBRARIES
            ${PropWare_PropWare_XMMC_LIBRARY}
            ${PropWare_Libpropeller_XMMC_LIBRARY}
            ${PropWare_Simple_XMMC_LIBRARY})
        set(PropWare_XMM-SINGLE_LIBRARIES
            ${PropWare_PropWare_XMM-SINGLE_LIBRARY}
            ${PropWare_Libpropeller_XMM-SINGLE_LIBRARY}
            ${PropWare_Simple_XMM-SINGLE_LIBRARY})
        set(PropWare_XMM-SPLIT_LIBRARIES
            ${PropWare_PropWare_XMM-SPLIT_LIBRARY}
            ${PropWare_Libpropeller_XMM-SPLIT_LIBRARY}
            ${PropWare_Simple_XMM-SPLIT_LIBRARY})

        file(READ "${PROPWARE_PATH}/version.txt" PropWare_VERSION)
        string(STRIP ${PropWare_VERSION} PropWare_VERSION)

        ##########################################
        # PropWare helper functions & macros
        ##########################################
        function (set_linker target)
            function (list_contains result request)
              set (${result} FALSE PARENT_SCOPE)
              foreach (listItem ${ARGN})
                if (${request} STREQUAL ${listItem})
                  set (${result} TRUE PARENT_SCOPE)
                endif ()
              endforeach ()
            endfunction ()

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
                        "PropWare requires linking with C or CXX. Please enable at least one of those languages")
                endif ()
            endif ()

            SET_TARGET_PROPERTIES("${target}"
                PROPERTIES
                LINKER_LANGUAGE
                ${linker_language})
        endfunction ()

        macro (set_compile_flags)
            if (AUTO_OTPIMIZATION)
                if (NOT AUTO_OTPIMIZATION_SET)
                    set(AUTO_OTPIMIZATION_SET 1)
                    set(COMMON_FLAGS "${COMMON_FLAGS} -Os")
                endif ()
            endif ()

            # Handle user options
            if (32_BIT_DOUBLES)
                if (NOT 32_BIT_DOUBLES_SET)
                    set(32_BIT_DOUBLES_SET 1)
                    set(COMMON_FLAGS "${COMMON_FLAGS} -m32bit-doubles")
                endif ()
            endif ()

            if (WARN_ALL)
                if (NOT WARN_ALL_SET)
                    set(WARN_ALL_SET 1)
                    set(COMMON_FLAGS "${COMMON_FLAGS} -Wall")
                endif ()
            endif ()

            if (AUTO_C_STD)
                if (NOT AUTO_C_STD_SET)
                    set(AUTO_C_STD_SET 1)
                    set(C_FLAGS "${C_FLAGS} -std=c99")
                endif ()
            endif ()

            if (AUTO_CXX_STD)
                if (NOT AUTO_CXX_STD_SET)
                    set(AUTO_CXX_STD_SET 1)
                    set(CXX_FLAGS "${CXX_FLAGS} -std=gnu++0x")
                endif ()
            endif ()

            # C++ Language features
            macro (add_language_feature_option option_name feature)
                if (${option_name})
                    if (NOT ${option_name}_SET)
                        set(${option_name}_SET 1)
                        set(CXX_FLAGS "${CXX_FLAGS} -f${feature}")
                    endif ()
                else ()
                    if (NOT ${option_name}_SET)
                        set(${option_name}_SET 1)
                        set(CXX_FLAGS "${CXX_FLAGS} -fno-${feature}")
                    endif ()
                endif ()
            endmacro ()
            add_language_feature_option(EXCEPTIONS exceptions)
            add_language_feature_option(RUNTIME_TYPE_INFORMATION rtti)
            add_language_feature_option(THREADSAFE_STATICS threadsafe-statics)

            # If no model is specified, we must choose a default so that the proper libraries can be linked
            if (NOT DEFINED MODEL)
                set(MODEL lmm)
            endif ()

            # XMM model is retroactively renamed xmm-split
            if ("${MODEL}" STREQUAL xmm)
                set(MODEL xmm-split)
            endif ()

            # Linker pruning is broken when used with the cog memory model. See the
            # following thread for a workaround:
            # http://forums.parallax.com/showthread.php/157878-Simple-blinky-program-and-linker-pruning
            string(TOLOWER "${MODEL}" MODEL_LOWERCASE)
            if (NOT((MODEL_LOWERCASE STREQUAL "cog")))
                if (AUTO_CUT_SECTIONS)
                    if (NOT AUTO_CUT_SECTIONS_SET)
                        set(AUTO_CUT_SECTIONS_SET 1)
                        set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
                        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
                    endif ()
                endif ()
            endif ()

            # Check if a deprecated variable name is set
            if (DEFINED CFLAGS OR DEFINED CXXFLAGS)
                if (NOT CFLAGS_CXXFLAGS_SET)
                    set(CFLAGS_CXXFLAGS_SET 1)
                    message(WARN ": The variables `CFLAGS` and `CXXFLAGS` have been replaced by `C_FLAGS` and `CXX_FLAGS`.")
                    set(C_FLAGS "${C_FLAGS} ${CFLAGS}")
                    set(CXX_FLAGS "${CXX_FLAGS} ${CXXFLAGS}")
                    set(CFLAGS )
                    set(CXXFLAGS )
                endif ()
            endif()

            if (NOT COMMON_FLAGS_SET)
                set(COMMON_FLAGS_SET 1)
                set(COMMON_FLAGS "-save-temps ${COMMON_FLAGS}")
            endif ()

            # Overwite the old flags
            set(CMAKE_ASM_FLAGS     "${COMMON_FLAGS}    ${ASM_FLAGS}")
            set(CMAKE_C_FLAGS       "${COMMON_FLAGS}    ${C_FLAGS}")
            set(CMAKE_CXX_FLAGS     "${COMMON_FLAGS}    ${CXX_FLAGS}")

            set(CMAKE_COGC_FLAGS    "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${COGC_FLAGS}")
            set(CMAKE_COGCXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${COGCXX_FLAGS}")
            set(CMAKE_ECOGC_FLAGS   "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${ECOGC_FLAGS}")
            set(CMAKE_ECOGCXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${ECOGCXX_FLAGS}")

            set(CMAKE_ASM_FLAGS     "${CMAKE_ASM_FLAGS} -m${MODEL}")
            set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}   -m${MODEL}")
            set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -m${MODEL}")
            set(CMAKE_DAT_FLAGS     "${MODEL}")
        endmacro ()

        function (add_prop_targets name target-suffix)
            if (DEFINED BOARD)
                set(BOARDFLAG -b${BOARD})
            endif()

            if (DEFINED GDB_BAUD)
                set(BAUDFLAG -b ${GDB_BAUD})
            elseif (DEFINED ENV{GDB_BAUD})
                set(BAUDFLAG -b $ENV{GDB_BAUD})
            endif ()

            # Add target for debugging (load to RAM and start GDB)
            add_custom_target(gdb${target-suffix}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -g &&
                    ${CMAKE_GDB} ${BAUDFLAG} $<TARGET_FILE:${name}>
                    DEPENDS ${name})

            # Add target for debugging (load to RAM and start terminal)
            add_custom_target(debug${target-suffix}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -t
                    DEPENDS ${name})

            # Add target for debugging in EEPROM (load to EEPROM and start terminal)
            add_custom_target(debug-eeprom${target-suffix}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -t -e
                DEPENDS ${name})

            # Add target for debugging from an SD card (load to SD card and start terminal)
            add_custom_target(debug-sd-cache${target-suffix}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} -z $<TARGET_FILE:${name}> -r -t -e
                DEPENDS ${name})

            # Add target for debugging from an SD card (load to SD card and start terminal)
            add_custom_target(debug-sd-loader${target-suffix}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} -l $<TARGET_FILE:${name}> -r -t -e
                DEPENDS ${name})

            # Add target for run (load to RAM, do not start terminal)
            add_custom_target(run-ram${target-suffix}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r
                    DEPENDS ${name})

            # Add target for run (load to EEPROM, do not start terminal)
            add_custom_target(run${target-suffix}
                    ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${name}> -r -e
                    DEPENDS ${name})

            # Add target for debugging from an SD card (load to SD card and start terminal)
            add_custom_target(run-sd-cache${target-suffix}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} -z $<TARGET_FILE:${name}> -r -e
                DEPENDS ${name})

            # Add target for debugging from an SD card (load to SD card and start terminal)
            add_custom_target(run-sd-loader${target-suffix}
                ${CMAKE_ELF_LOADER} ${BOARDFLAG} -l $<TARGET_FILE:${name}> -r -e
                DEPENDS ${name})
        endfunction ()

        macro (_pw_create_executable name suffix src1)
            # Set flags
            set_compile_flags()
            include_directories(${PropWare_INCLUDE_DIR})

            # Create the binary
            add_executable("${name}" "${src1}" ${ARGN})

            # Link it with the appropriate static libraries (and use C template for linking)
            string(TOUPPER ${MODEL} _PW_UPPER_MODEL)
            target_link_libraries(${name} ${PropWare_${_PW_UPPER_MODEL}_LIBRARIES})
            set_linker(${name})

            # Create propeller-load targets
            add_prop_targets(${name} "${suffix}")
        endmacro ()

        macro (create_executable name src1)
            _pw_create_executable("${name}" "-${name}" "${src1}" ${ARGN})
        endmacro()

        macro (create_simple_executable name src1)
            if (PROPWARE_MAIN_PACKAGE)
                _pw_create_executable("${name}" "-${name}" "${src1}" ${ARGN})
            else ()
                _pw_create_executable("${name}" "" "${src1}" ${ARGN})
            endif ()
        endmacro()

        macro(create_library name src1)
            add_library(${name} "${src1}" ${ARGN})
            set_compile_flags()
            include_directories(${PropWare_INCLUDE_DIR})
            set_linker(${name})
        endmacro()

        function(spin2cpp source output_var_name)
            get_filename_component(SOURCE_PATH "${source}" ABSOLUTE)

            # Find output files
            execute_process(COMMAND "${SPIN2CPP_COMMAND}" --files "${SOURCE_PATH}"
                OUTPUT_VARIABLE FILES_STRING
                RESULT_VARIABLE SPIN2CPP_DEPENDS_CODE
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (SPIN2CPP_DEPENDS_CODE)
                message(FATAL_ERROR "Spin2cpp failed to report dependencies. Exit code ${SPIN2CPP_DEPENDS_CODE}")
            endif ()

            # Convert output files from newline-separated list to CMake list
            string(REPLACE "\r" "" OUTPUT_FILE_NAMES "${FILES_STRING}")
            string(REPLACE "\n" ";" OUTPUT_FILE_NAMES "${OUTPUT_FILE_NAMES}")
            foreach (file_name IN LISTS OUTPUT_FILE_NAMES)
                list(APPEND ALL_OUTPUT_FILES "${CMAKE_CURRENT_BINARY_DIR}/${file_name}")
            endforeach ()

            # Only save new files in the "output list" variable and add to the clean target
            foreach (file_path IN LISTS ALL_OUTPUT_FILES)
                list(FIND FILES_GENERATED_IN_DIRECTORY ${file_path} INDEX)
                if ("-1" STREQUAL INDEX)
                    list(APPEND FILES_GENERATED_IN_DIRECTORY "${file_path}")
                    list(APPEND UNIQUE_OUTPUT_FILES "${file_path}")
                endif ()
            endforeach ()
            set(FILES_GENERATED_IN_DIRECTORY ${FILES_GENERATED_IN_DIRECTORY} PARENT_SCOPE)
            set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${FILES_GENERATED_IN_DIRECTORY}")

            if (UNIQUE_OUTPUT_FILES)
                if (ARGN)
                    set(MAIN_FLAG "--main")
                endif ()
                include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
                add_custom_command(OUTPUT ${UNIQUE_OUTPUT_FILES}
                    COMMAND "${SPIN2CPP_COMMAND}"
                    ARGS --gas ${MAIN_FLAG} "${SOURCE_PATH}"
                    MAIN_DEPENDENCY "${SOURCE_PATH}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    COMMENT "Converting ${source} to C++")
            endif ()
            set(${output_var_name} ${UNIQUE_OUTPUT_FILES} PARENT_SCOPE)
        endfunction()

        # TODO: Add build system documentation for testing
        enable_testing()
        add_custom_target(test-all COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
        macro(create_test target src1)
            create_executable(${target} "${src1}" ${ARGN})
            add_test(NAME ${target}
                COMMAND ${CMAKE_ELF_LOADER} ${BOARDFLAG} $<TARGET_FILE:${target}> -r -t -q)
            add_dependencies(test-all ${target})
            add_custom_target(test-${target}
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ${target}
                DEPENDS ${target})
        endmacro()
    endif ()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(PropWare
        FOUND_VAR PropWare_FOUND
        REQUIRED_VARS
            PROPWARE_PATH
            PropWare_INCLUDE_DIR
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
        PropWare_Libpropeller_COG_LIBRARY
        PropWare_Libpropeller_CMM_LIBRARY
        PropWare_Libpropeller_LMM_LIBRARY
        PropWare_Libpropeller_XMMC_LIBRARY
        PropWare_Libpropeller_XMM-SPLIT_LIBRARY
        PropWare_Libpropeller_XMM-SINGLE_LIBRARY
        PropWare_Simple_COG_LIBRARY
        PropWare_Simple_CMM_LIBRARY
        PropWare_Simple_LMM_LIBRARY
        PropWare_Simple_XMMC_LIBRARY
        PropWare_Simple_XMM-SPLIT_LIBRARY
        PropWare_Simple_XMM-SINGLE_LIBRARY
    )
endif()
