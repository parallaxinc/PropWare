file(GLOB_RECURSE LIBPROPELLER_OBJECTS_WITH_DUPS
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.S
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.cpp)

# Remove symlinks
foreach (file IN LISTS LIBPROPELLER_OBJECTS_WITH_DUPS)
    if (NOT IS_SYMLINK ${file})
        list(APPEND LIBPROPELLER_OBJECTS "${file}")
    endif ()
endforeach ()

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
