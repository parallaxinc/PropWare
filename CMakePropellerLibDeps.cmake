# Create library dependencies based on CMake options
function(setLibraryDependencies executable)

    if (LINK_LIBPROPELLER)
        target_link_libraries(${executable} ${PROPWARE_PATH}/bin/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
    endif ()

    if (LINK_SIMPLE)
        target_link_libraries(${executable} ${PROPWARE_PATH}/bin/simple/${MODEL}/libSimple_${MODEL}.a)
        include_directories(SYSTEM ${PROPWARE_PATH}/simple)
    endif ()

    if (NOT((MODEL STREQUAL "cog") OR (MODEL STREQUAL "COG")))
        if (LINK_PROPWARE)
            target_link_libraries(${executable} ${PROPWARE_PATH}/bin/PropWare/${MODEL}/libPropWare_${MODEL}.a)
        endif ()
    endif ()

    if (LINK_TINY)
        target_link_libraries(${executable} tiny)
    endif ()

    if (LINK_MATH)
        target_link_libraries(${executable} m)
    endif ()

endfunction()
