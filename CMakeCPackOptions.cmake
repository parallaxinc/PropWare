# This file is configured at cmake time, and loaded at cpack time.
# To pass variables to cpack from cmake, they must be configured
# in this file.

if (CPACK_GENERATOR MATCHES "ZIP")
    set(CPACK_COMPONENTS_ALL propware win_cmake cmake osx_cmake)
elseif (CPACK_GENERATOR MATCHES "NSIS")
    set(CPACK_COMPONENTS_ALL propware win_cmake)
else ()
    set(CPACK_COMPONENTS_ALL propware cmake)
endif ()
