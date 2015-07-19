file(GLOB_RECURSE LIBPROPELLER_OBJECTS
    ${PROJECT_SOURCE_DIR}/libpropeller/*.S)
list(APPEND LIBPROPELLER_OBJECTS
    ${PROJECT_SOURCE_DIR}/libpropeller/numbers/numbers.cpp)

add_subdirectory(source)
