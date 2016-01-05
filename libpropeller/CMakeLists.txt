file(GLOB_RECURSE LIBPROPELLER_OBJECTS_WITH_DUPS
    ${CMAKE_CURRENT_LIST_DIR}/[!u]*/*.S
    ${CMAKE_CURRENT_LIST_DIR}/[!u]*/*.cpp)

# Remove duplicates (CMake's built-in function doesn't recognize the duplicates because they are in different folders)
foreach (file IN LISTS LIBPROPELLER_OBJECTS_WITH_DUPS)
    get_filename_component(FILENAME "${file}" NAME)
    if (NOT (FILENAME IN_LIST FILENAMES))
        list(APPEND FILENAMES "${FILENAME}")
        list(APPEND LIBPROPELLER_OBJECTS "${file}")
    endif ()
endforeach ()

add_subdirectory(source)
