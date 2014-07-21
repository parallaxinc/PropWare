# Create library dependencies
add_library(LIBPROPELLER_LIB
        STATIC IMPORTED)
add_library(SIMPLE_LIB
        STATIC IMPORTED)
add_library(PROPWARE_LIB
        STATIC IMPORTED)

# Set locations for library dependencies
set_target_properties(LIBPROPELLER_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
set_target_properties(SIMPLE_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/simple/${MODEL}/libSimple_${MODEL}.a)
set_target_properties(PROPWARE_LIB
        PROPERTIES
        IMPORTED_LOCATION
        ${PROPWARE_PATH}/PropWare/${MODEL}/libPropWare_${MODEL}.a)

# Add links to all four major components
target_link_libraries(${PROJECT_NAME}
        LIBPROPELLER_LIB
        SIMPLE_LIB
        PROPWARE_LIB
        tiny)

# Only add these custom targets if we're not compiling the PropWare library
if (NOT DEFINED PROPWARE_MAIN_PACKAGE)
    if (DEFINED BOARD)
        set(BOARDFLAG -b${BOARD})
    endif(DEFINED BOARD)

    # Add target for run (load to RAM and start terminal)
    add_custom_target(run
            ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${PROJECT_NAME}.elf -r -t
            DEPENDS ${CMAKE_PROJECT_NAME}.elf)

    # Add target for install (load to EEPROM and start terminal)
#    add_custom_target(install
#            ${CMAKE_ELF_LOADER} ${BOARDFLAG} ${PROJECT_NAME}.elf -r -t -e
#            DEPENDS ${CMAKE_PROJECT_NAME}.elf)
ENDIF(NOT DEFINED PROPWARE_MAIN_PACKAGE)
