include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

# Create library dependencies
add_library(LIBPROPELLER_LIB
        STATIC IMPORTED)
add_library(SIMPLE_LIB
        STATIC IMPORTED)
add_library(PROPWARE_LIB
        STATIC IMPORTED)

include_directories("${PROPWARE_PATH}")

# Set locations for library dependencies
set_target_properties(LIBPROPELLER_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/bin/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
set_target_properties(SIMPLE_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/bin/simple/${MODEL}/libSimple_${MODEL}.a)
set_target_properties(PROPWARE_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/bin/PropWare/${MODEL}/libPropWare_${MODEL}.a)

# Add links to all four major components
target_link_libraries(${PROJECT_NAME}
        LIBPROPELLER_LIB
        SIMPLE_LIB
        PROPWARE_LIB
        tiny)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

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
endif()
