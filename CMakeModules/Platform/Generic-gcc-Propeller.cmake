message(STATUS "LOADED: Generic-gcc-Propeller.cmake")

################################################################################
### Set Compile Flags
################################################################################

# Create standard flags
set(ASMFLAGS_INIT "")
set(CFLAGS_INIT "")
set(CSTANDARD "")
set(CXXFLAGS_INIT "-fno-threadsafe-statics -fno-rtti")

set(CMAKE_ASM_FLAGS_INIT "${ASMFLAGS_INIT}")
set(CMAKE_C_FLAGS_INIT "${CFLAGS_INIT}")
set(CMAKE_CXX_FLAGS_INIT "${CFLAGS_INIT} ${CXXFLAGS_INIT}")
set(CMAKE_COGC_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -mcog -xc -r")
set(CMAKE_ECOGC_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -mcog -xc -r")
set(CMAKE_COGCXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -mcog -xc++ -r")
set(CMAKE_ECOGCXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -mcog -xc++ -r")

# File names
set(CMAKE_STATIC_LIBRARY_PREFIX lib)
set(CMAKE_STATIC_LIBRARY_SUFFIX .a)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

################################################################################
### Flags
################################################################################
# Set flags
set(CMAKE_AR_FLAGS "cr")

################################################################################
### Compilers
################################################################################
set(CMAKE_C_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> ${CMAKE_C_FLAGS_INIT} <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_CXX_COMPILE_OBJECT
"<CMAKE_CXX_COMPILER> <DEFINES> ${CMAKE_CXX_FLAGS_INIT} <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_ASM_COMPILE_OBJECT
"<CMAKE_ASM_COMPILER> <DEFINES> ${CMAKE_ASM_FLAGS_INIT} <FLAGS> -o <OBJECT> -c <SOURCE>")

################################################################################
### Linkers
################################################################################
set(CMAKE_C_ARCHIVE_CREATE
"<CMAKE_AR> cr <TARGET> <LINK_FLAGS> <OBJECTS>")

set(CMAKE_C_ARCHIVE_APPEND
"<CMAKE_AR> r  <TARGET> <LINK_FLAGS> <OBJECTS>")

set(CMAKE_C_ARCHIVE_FINISH
"<CMAKE_RANLIB> <TARGET>")

set(CMAKE_C_LINK_EXECUTABLE
"<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o<TARGET> <OBJECTS> <LINK_LIBRARIES>")

include_directories(${PROPWARE_PATH})

mark_as_advanced(
    CMAKE_BUILD_TYPE
    CMAKE_INSTALL_PREFIX
)
