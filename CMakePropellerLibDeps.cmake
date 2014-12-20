# Create library dependencies based on CMake options
function(setLibraryDependencies executable)

    if (LINK_LIBPROPELLER)
        add_library(LIBPROPELLER_LIB STATIC IMPORTED)
        set_target_properties(LIBPROPELLER_LIB
                PROPERTIES
                IMPORTED_LOCATION
                ${PROPWARE_PATH}/bin/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
        target_link_libraries(${executable} LIBPROPELLER_LIB)
    endif ()
    
    if (LINK_SIMPLE)
        add_library(SIMPLE_LIB STATIC IMPORTED)
        set_target_properties(SIMPLE_LIB
                PROPERTIES
                IMPORTED_LOCATION
                ${PROPWARE_PATH}/bin/simple/${MODEL}/libSimple_${MODEL}.a)
        target_link_libraries(${executable}
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
            target_link_libraries(${executable} PROPWARE_LIB)
        endif ()
    endif ()
    
    if (LINK_TINY)
        target_link_libraries(${executable} tiny)
    endif ()

endfunction()
