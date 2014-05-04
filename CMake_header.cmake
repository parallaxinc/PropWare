set (CMAKE_BUILD_TYPE None)

if (NOT DEFINED BOARD)
    set(BOARD $ENV{PROPELLER_LOAD_BOARD})
endif (NOT DEFINED BOARD)

if (DEFINED BOARD)
    set(BOARDFLAG -b${BOARD})
endif(DEFINED BOARD)

include(${PROPWARE_PATH}/PropellerToolchain.cmake)

# Check for a linker script
if (DEFINED LDSCRIPT)
    set(LDFLAGS "${LDFLAGS} -T '${LDSCRIPT}'")
endif (DEFINED LDSCRIPT)

# Add library search paths
set(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS}")

# Remove dynamic link flag
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

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
target_link_libraries(${CMAKE_PROJECT_NAME}.elf
        LIBPROPELLER_LIB
        SIMPLE_LIB
        PROPWARE_LIB
        tiny)