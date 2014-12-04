include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

################################################################################
# Create library dependencies

if (LINK_LIBPROPELLER)
    add_library(LIBPROPELLER_LIB STATIC IMPORTED)
    set_target_properties(LIBPROPELLER_LIB
            PROPERTIES
            IMPORTED_LOCATION
            ${PROPWARE_PATH}/bin/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
    target_link_libraries(${PROJECT_NAME} LIBPROPELLER_LIB)
endif ()

if (LINK_SIMPLE)
    add_library(SIMPLE_LIB STATIC IMPORTED)
    set_target_properties(SIMPLE_LIB
            PROPERTIES
            IMPORTED_LOCATION
            ${PROPWARE_PATH}/bin/simple/${MODEL}/libSimple_${MODEL}.a)
    target_link_libraries(${PROJECT_NAME}
            SIMPLE_LIB)
    include_directories(SYSTEM ${PROPWARE_PATH}/simple)
endif ()

if (NOT((MODEL STREQUAL "cog") OR (MODEL STREQUAL "COG")))
    if (LINK_PROPWARE)
        add_library(PROPWARE_LIB STATIC IMPORTED)
        set_target_properties(PROPWARE_LIB
                PROPERTIES
                IMPORTED_LOCATION
                ${PROPWARE_PATH}/bin/PropWare/${MODEL}/libPropWare_${MODEL}.a)
        target_link_libraries(${PROJECT_NAME} PROPWARE_LIB)
    endif ()
endif ()

if (LINK_TINY)
    target_link_libraries(${PROJECT_NAME} tiny)
endif ()

# End library dependencies
################################################################################

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

SET_TARGET_PROPERTIES(${PROJECT_NAME}
    PROPERTIES
    LINKER_LANGUAGE
    ${linker_language})
include_directories(SYSTEM ${PROPWARE_PATH})

# Only add these custom targets if we're not compiling the PropWare library
if (NOT DEFINED PROPWARE_MAIN_PACKAGE)
    if (DEFINED BOARD)
        set(BOARDFLAG -b${BOARD})
    endif()

    # Add target for run (load to RAM and start terminal)
    add_custom_target(debug
            ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${PROJECT_NAME}.elf -r -t
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})

    # Add target for run (load to EEPROM, do not start terminal)
    add_custom_target(run
            ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${PROJECT_NAME}.elf -r -e
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})
endif ()
