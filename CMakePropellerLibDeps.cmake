# Create library dependencies based on CMake options
function(setLibraryDependencies executable)

    if (LINK_LIBPROPELLER)
        if (DEFINED PROPWARE_MAIN_PACKAGE)
            target_link_libraries(${executable} Libpropeller_${MODEL})
        else ()
            target_link_libraries(${executable} ${PROPWARE_PATH}/bin/libpropeller/source/${MODEL}/libLibpropeller_${MODEL}.a)
        endif ()
    endif ()

    if (LINK_SIMPLE)
        if (DEFINED PROPWARE_MAIN_PACKAGE)
            target_link_libraries(${executable} Simple_${MODEL})
        else ()
            target_link_libraries(${executable} ${PROPWARE_PATH}/bin/simple/${MODEL}/libSimple_${MODEL}.a)
        endif ()


        if (PROJECT_NAME STREQUAL "PropWare")
            include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/simple)
        else ()
            include_directories(SYSTEM ${PROPWARE_PATH}/simple)
        endif ()
    endif ()

    if (NOT((MODEL STREQUAL "cog") OR (MODEL STREQUAL "COG")))
        if (LINK_PROPWARE)

            if (DEFINED PROPWARE_MAIN_PACKAGE)
                target_link_libraries(${executable} PropWare_${MODEL})
            else ()
                target_link_libraries(${executable} ${PROPWARE_PATH}/bin/PropWare/${MODEL}/libPropWare_${MODEL}.a)
            endif ()
        endif ()
    endif ()

    if (LINK_TINY)
        target_link_libraries(${executable} tiny)
    endif ()

    if (LINK_MATH)
        target_link_libraries(${executable} m)
    endif ()

endfunction()
