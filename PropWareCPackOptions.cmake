# This file is configured at cmake time, and loaded at cpack time.
# To pass variables to cpack from cmake, they must be configured
# in this file.

if (CPACK_GENERATOR MATCHES "ZIP")
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE OFF)
    set(CPACK_COMPONENTS_IGNORE_GROUPS ON)
    set(CPACK_COMPONENTS_ALL propware examples)
elseif (CPACK_GENERATOR MATCHES "NSIS")
    set(CPACK_COMPONENTS_ALL propware examples win_cmake win_spin2cpp)
elseif (CPACK_GENERATOR MATCHES "(DEB|RPM)")
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
    set(CPACK_COMPONENTS_ALL propware examples linux_cmakemodules)
elseif (CPACK_GENERATOR MATCHES "PackageMaker")
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
    set(CPACK_COMPONENTS_ALL propware examples macosx_cmakemodules)
endif ()
