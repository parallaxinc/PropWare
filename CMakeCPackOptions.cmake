# This file is configured at cmake time, and loaded at cpack time.
# To pass variables to cpack from cmake, they must be configured
# in this file.


set(CPACK_COMPONENT_propware_DISPLAY_NAME "Headers/Libraries")
set(CPACK_COMPONENT_propware_DESCRIPTION  "Headers and static libraries for PropWare, Simple and libpropeller")

set(CPACK_COMPONENT_cmake_DISPLAY_NAME     "CMake")
set(CPACK_COMPONENT_cmake_DESCRIPTION      "Complete CMake installation with additional files for easy Propeller development")
set(CPACK_COMPONENT_win_cmake_DISPLAY_NAME "CMake")
set(CPACK_COMPONENT_win_cmake_DESCRIPTION  "Complete CMake installation with additional files for easy Propeller development")

if (CPACK_GENERATOR MATCHES "ZIP")
    set(CPACK_COMPONENTS_ALL propware win_cmake cmake)
elseif (CPACK_GENERATOR MATCHES "NSIS")
    set(CPACK_COMPONENTS_ALL propware win_cmake)
else ()
    set(CPACK_COMPONENTS_ALL propware cmake)
endif ()
