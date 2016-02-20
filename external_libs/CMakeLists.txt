####################################################################
# Find libpropeller sources
####################################################################
file(GLOB_RECURSE LIBPROPELLER_OBJECTS_WITH_DUPS
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.S
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.cpp)

# Remove symlinks
foreach (file IN LISTS LIBPROPELLER_OBJECTS_WITH_DUPS)
    if (NOT IS_SYMLINK ${file})
        list(APPEND LIBPROPELLER_OBJECTS "${file}")
    endif ()
endforeach ()

####################################################################
# Find Simple sources
####################################################################
set(SIMPLE_ROOT "${PROJECT_SOURCE_DIR}/propsideworkspace/Learn/Simple Libraries")

set(SOURCE_FILE_EXTENSIONS
    *.c
    *.C
    *.cpp
    *.cxx
    *.cc
    *.s
    *.S
    *.dat
    *.cogc
    *.cogcxx
    *.cogcpp
    *.ecogc
    *.ecogcxx
    *.ecogcpp
    *.spin)
set(HEADER_FILE_EXTENSIONS
    *.h
    *.H
    *.hpp
    *.HPP
    *.Hpp)

function(subdirlist result curdir)
    file(GLOB children RELATIVE "${curdir}" "${curdir}/*")
    set(dirlist "")
    foreach (child IN LISTS children)
        if (IS_DIRECTORY "${curdir}/${child}")
            list(APPEND dirlist "${child}")
        endif ()
    endforeach ()
    set(${result} ${dirlist} PARENT_SCOPE)
endfunction()

function(compute_demo_filenames result library)
    set(cdf_FILES "")
    get_filename_component(library_name "${library}" NAME)
    foreach (extension ".c" ".h")
        list(APPEND cdf_FILES "${library}/${library_name}${extension}")
    endforeach ()
    set(${result} ${cdf_FILES} PARENT_SCOPE)
endfunction()

# Get most categories
subdirlist(CATEGORIES "${SIMPLE_ROOT}")

# Add all the robots
list(REMOVE_ITEM CATEGORIES Robotics)
subdirlist(ROBOTS "${SIMPLE_ROOT}/Robotics")
foreach (robot IN LISTS ROBOTS)
    list(APPEND CATEGORIES "Robotics/${robot}")
endforeach ()

# Find the libraries in each category
foreach (category IN LISTS CATEGORIES)
    set(category "${SIMPLE_ROOT}/${category}")
    subdirlist(CATEGORY_LIBRARIES ${category})
    foreach (new_library IN LISTS CATEGORY_LIBRARIES)
        list(APPEND LIBRARIES "${category}/${new_library}")
    endforeach ()
endforeach ()

# Manually add simpletools
list(APPEND LIBRARIES "${SIMPLE_ROOT}/Utility/libsimpletools/source")

foreach (library IN LISTS LIBRARIES)
    compute_demo_filenames(DEMO_FILES "${library}")

    set(GLOB_PATTERNS "")
    foreach (extension IN LISTS SOURCE_FILE_EXTENSIONS)
        list(APPEND GLOB_PATTERNS "${library}/*${extension}")
    endforeach ()
    file(GLOB ALL_SOURCE_FILES_IN_LIB ${GLOB_PATTERNS})
    foreach (file IN LISTS ALL_SOURCE_FILES_IN_LIB)
        if (NOT "${file}" IN_LIST DEMO_FILES)
            list(APPEND SIMPLE_OBJECTS "${file}")
        endif ()
    endforeach ()

    set(GLOB_PATTERNS "")
    foreach (extension IN LISTS HEADER_FILE_EXTENSIONS)
        list(APPEND GLOB_PATTERNS "${library}/*${extension}")
    endforeach ()
    file(GLOB ALL_HEADER_FILES_IN_LIB ${GLOB_PATTERNS})
    foreach (file IN LISTS ALL_HEADER_FILES_IN_LIB)
        if (NOT "${file}" IN_LIST DEMO_FILES)
            list(APPEND SIMPLE_HEADERS "${file}")
            list(APPEND SIMPLE_OBJECTS "${file}") # Not necessary, but provides better help in CLion
        endif ()
    endforeach ()
endforeach ()

list(SORT SIMPLE_OBJECTS)

# Add simpletools.h to the list of libraries, which now works as our list of include directories
list(APPEND LIBRARIES "${SIMPLE_ROOT}/Utility/libsimpletools")

# Ensure the include directories are added for examples and unit tests too. This is done twice because the
# "PARENT_SCOPE" option _only_ sets the variable in the parent scope (it ignores the current scope)
set(PropWare_INCLUDE_DIR ${PropWare_INCLUDE_DIR} ${LIBRARIES})
set(PropWare_INCLUDE_DIR ${PropWare_INCLUDE_DIR} PARENT_SCOPE)

install(FILES ${SIMPLE_HEADERS}
    DESTINATION PropWare/include
    COMPONENT propware)

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
