set(CPACK_GENERATOR
    ZIP
)

if (PACKAGE_LINUX OR PACKAGE_RPI2)
    list(APPEND CPACK_GENERATOR RPM DEB)
endif ()
if (PACKAGE_WIN32)
    list(APPEND CPACK_GENERATOR NSIS)
endif ()

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR}"
    "${PROJECT_NAME}"
    ALL
    /
)
set(CPACK_PROJECT_URL                           "http://david.zemon.name/PropWare")
set(CPACK_PACKAGE_VENDOR                        "David Zemon")
set(CPACK_PACKAGE_CONTACT                       "David Zemon <david@zemon.name>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY           "C++ objects and CMake build system for Parallax Propeller")
set(CPACK_RESOURCE_FILE_README                  "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_CMAKE_GENERATOR                       "Unix Makefiles")
set(CPACK_PACKAGE_VERSION_MAJOR                 ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR                 ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH                 ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
if (PROJECT_VERSION_TWEAK)
    set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}.${PROJECT_VERSION_TWEAK})
endif ()

# NSIS Specific
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL  ON)
set(CPACK_NSIS_HELP_LINK                        "${CPACK_PROJECT_URL}")
set(CPACK_NSIS_URL_INFO_ABOUT                   "${CPACK_PROJECT_URL}")
set(CPACK_NSIS_CONTACT                          "${CPACK_PACKAGE_CONTACT}")
set(CPACK_NSIS_MUI_ICON                         "${PROJECT_SOURCE_DIR}/docs/images/PropWare_Logo.ico")
set(CPACK_NSIS_MUI_UNIICON                      "${PROJECT_SOURCE_DIR}/docs/images/PropWare_Logo.ico")
set(CPACK_NSIS_INSTALL_ROOT                     C:)
set(CPACK_PACKAGE_INSTALL_DIRECTORY             PropWare) # Required because default contains spaces and version number
set(CPACK_NSIS_EXECUTABLES_DIRECTORY            .)
set(CPACK_PACKAGE_EXECUTABLES
    "${CUSTOM_WIN32_CMAKE_INSTALL_DIR}\\\\bin\\\\cmake" CMake
    "${CUSTOM_WIN32_CMAKE_INSTALL_DIR}\\\\bin\\\\cmake-gui" "CMake GUI"
    "${CUSTOM_WIN32_CMAKE_INSTALL_DIR}\\\\bin\\\\ctest" CTest
    "${CUSTOM_WIN32_CMAKE_INSTALL_DIR}\\\\bin\\\\cpack" CPack)

# Debian Specific
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE               "${CPACK_PROJECT_URL}")
set(CPACK_DEBIAN_PACKAGE_SECTION                devel)
set(CPACK_DEBIAN_PACKAGE_PRIORITY               optional)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS              ON)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS             "make")
set(CPACK_DEBIAN_PACKAGE_REPLACES               "cmake")
set(CPACK_DEBIAN_FILE_NAME                      DEB-DEFAULT)

# RPM Specific
set(CPACK_RPM_PACKAGE_URL                       "${CPACK_PROJECT_URL}")
set(CPACK_RPM_PACKAGE_SUGGESTS                  "make")
set(CPACK_RPM_PACKAGE_OBSOLETES                 "cmake")
set(CPACK_PACKAGE_RELOCATABLE                   ON)
set(CPACK_RPM_FILE_NAME                         RPM-DEFAULT)

# Components
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_NSIS_COMPONENT_INSTALL ON)

set(CPACK_COMPONENT_PROPWARE_DISPLAY_NAME           "Headers/Libraries")
set(CPACK_COMPONENT_PROPWARE_DESCRIPTION            "Headers and static libraries for PropWare, Simple and libpropeller")
set(CPACK_COMPONENT_CMAKE_DISPLAY_NAME              "CMake")
set(CPACK_COMPONENT_CMAKE_DESCRIPTION               "Complete CMake installation with additional files for easy Propeller development")
set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME           "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION            "Examples projects for each of PropWare, Simple and libpropeller using the PropWare build system")
set(CPACK_COMPONENT_WIN_CMAKE_DISPLAY_NAME          "${CPACK_COMPONENT_CMAKE_DISPLAY_NAME}")
set(CPACK_COMPONENT_WIN_CMAKE_DESCRIPTION           "${CPACK_COMPONENT_CMAKE_DESCRIPTION}")
set(CPACK_COMPONENT_LINUX_SPIN2CPP_DISPLAY_NAME     "Spin2Cpp")
set(CPACK_COMPONENT_LINUX_SPIN2CPP_DESCRIPTION      "Compile Spin files for use in C/C++ projects")
set(CPACK_COMPONENT_WIN_SPIN2CPP_DISPLAY_NAME       "${CPACK_COMPONENT_LINUX_SPIN2CPP_DISPLAY_NAME}")
set(CPACK_COMPONENT_WIN_SPIN2CPP_DESCRIPTION        "${CPACK_COMPONENT_LINUX_SPIN2CPP_DESCRIPTION}")

configure_file("${PROJECT_SOURCE_DIR}/CMakeCPackOptions.cmake.in"
    "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CMakeCPackOptions.cmake")
include(CPack)
