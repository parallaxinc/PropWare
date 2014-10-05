if (DEFINED PROPWARE_PATH)
    set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)
else ()
    set(CMAKE_TOOLCHAIN_FILE $ENV{PROPWARE_PATH}/PropellerToolchain.cmake)
endif ()

# Printf with float support
option(PROPWARE_PRINT_FLOAT "Enable floating point support in PropWare's printf method" OFF)

# Linker options
option(LINK_LIBPROPELLER "Link with libpropeller library" ON)
option(LINK_SIMPLE "Link with Parallax's Simple library" ON)
option(LINK_PROPWARE "Link with PropWare library" ON)
option(LINK_TINY "Link with Parllax's tiny library (deprecated)" OFF)

# Compilation options
option(32_BIT_DOUBLES "Set all doubles to 32-bits (-m32bit-doubles)" ON)
option(WARN_ALL "Turn on all compiler warnings (-Wall)" ON)
option(AUTO_C_STD "Set C standard to c99 (-std=c99)" ON)
option(AUTO_CXX_STD "Set C++ standard to gnu++0x (-std=gnu++0x)" ON)

# Optimize size option
option(AUTO_CUT_SECTIONS "Cut out unused code (Compile: -ffunction-sections -fdata-sections; Link: --gc-sections)" ON)

mark_as_advanced(CMAKE_TOOLCHAIN_FILE)