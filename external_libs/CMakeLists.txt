file(GLOB_RECURSE LIBPROPELLER_OBJECTS_WITH_DUPS
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.S
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.cpp)

# Remove duplicates (CMake's built-in function doesn't recognize the duplicates because they are in different folders)
foreach (file IN LISTS LIBPROPELLER_OBJECTS_WITH_DUPS)
    get_filename_component(FILENAME "${file}" NAME)
    if (NOT (FILENAME IN_LIST FILENAMES))
        list(APPEND FILENAMES "${FILENAME}")
        list(APPEND LIBPROPELLER_OBJECTS "${file}")
    endif ()
endforeach ()

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
