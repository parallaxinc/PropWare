# Turn off warnings for code that I don't own
set(WARN_ALL OFF)

####################################################################
# Find libpropeller sources
file(GLOB_RECURSE LIBPROPELLER_OBJECTS_WITH_DUPS
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.S
    ${PROJECT_SOURCE_DIR}/libpropeller/libpropeller/[!u]*/*.cpp)

# Remove symlinks
foreach (file IN LISTS LIBPROPELLER_OBJECTS_WITH_DUPS)
    if (NOT IS_SYMLINK ${file})
        list(APPEND LIBPROPELLER_OBJECTS "${file}")
    endif ()
endforeach ()

set(LIBPROPELLER_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libpropeller")
####################################################################

####################################################################
# Find libPropelleruino sources
file(GLOB LIBPROPELLERUINO_OBJECTS
    ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/*.cpp
    ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/*.c)

list(REMOVE_ITEM LIBPROPELLERUINO_OBJECTS

    # Remove the two files that contain main functions
    ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/main.cpp
    ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/Arduino.cpp

    # And ignore Wire.cpp because its include headers are broken (missing file)
    ${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino/Wire.cpp
)

set(LIBPROPELLERUINO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libArduino/libPropelleruino")
####################################################################

####################################################################
include(./ParseSimpleIDEMetaData.cmake)

# Find Simple sources
set(SIMPLE_ROOT "${PROJECT_SOURCE_DIR}/Simple-Libraries/Learn/Simple Libraries")

# Find SimpleIDE project files
file(GLOB_RECURSE SIMPLE_IDE_PROJECT_FILES "${SIMPLE_ROOT}/*.side")

# Find Simple's header files
file(GLOB_RECURSE SIMPLE_HEADERS "${SIMPLE_ROOT}/*.h")

# Find include directories
set(SIMPLE_INCLUDE_DIRS )
foreach(project_file IN LISTS SIMPLE_IDE_PROJECT_FILES)
    get_filename_component(LIBRARY_ROOT "${project_file}" DIRECTORY)
    list(APPEND SIMPLE_INCLUDE_DIRS "${LIBRARY_ROOT}")
endforeach()

install(FILES ${SIMPLE_HEADERS}
    DESTINATION share/PropWare/include
    COMPONENT propware)
####################################################################

####################################################################
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ArduinoJson/src/
    DESTINATION share/PropWare/include/c++
    COMPONENT propware)
####################################################################

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
