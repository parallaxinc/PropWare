################################################################################
### Set Compile Flags
################################################################################

# Create standard flags
set(ASMFLAGS_INIT "")
set(CFLAGS_INIT "")
set(CSTANDARD "")
set(CXXFLAGS_INIT "")

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
set(CMAKE_EXECUTABLE_SUFFIX .elf CACHE STRING "Executable suffix")

################################################################################
### Flags
################################################################################
# Set flags
set(CMAKE_AR_FLAGS "cr")

################################################################################
### Compilers
################################################################################
if (CMAKE_VERSION VERSION_EQUAL "3.4.0" OR CMAKE_VERSION VERSION_GREATER "3.4.0")
    set(INCLUDES "<INCLUDES>")
endif ()
set(CMAKE_C_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> ${CMAKE_C_FLAGS_INIT} <FLAGS> ${INCLUDES} -o <OBJECT> -c <SOURCE>")

set(CMAKE_CXX_COMPILE_OBJECT
"<CMAKE_CXX_COMPILER> <DEFINES> ${CMAKE_CXX_FLAGS_INIT} <FLAGS> ${INCLUDES} -o <OBJECT> -c <SOURCE>")

set(CMAKE_ASM_COMPILE_OBJECT
"<CMAKE_ASM_COMPILER> <DEFINES> ${CMAKE_ASM_FLAGS_INIT} <FLAGS> ${INCLUDES} -o <OBJECT> -c <SOURCE>")

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
"<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o<TARGET> <OBJECTS> <LINK_LIBRARIES>"
"${CMAKE_COMMAND} -DBINARY=<TARGET> -DSIZE_EXE=${CMAKE_ELF_SIZE} -P ${ELF_SIZER}")

mark_as_advanced(
    CMAKE_BUILD_TYPE
    CMAKE_INSTALL_PREFIX
)
