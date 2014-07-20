################################################################################
### !!! IMPORTANT !!!
### This file must exist in the cmake installation direction under
### Modules/Platform/
################################################################################
# File names
set(CMAKE_STATIC_LIBRARY_PREFIX lib)
set(CMAKE_STATIC_LIBRARY_SUFFIX .a)
set(CMAKE_C_OUTPUT_EXTENSION .o)
set(CMAKE_CXX_OUTPUT_EXTENSION .o) # TODO: Why doesn't this work?
set(CMAKE_ASM_OUTPUT_EXTENSION .o) # TODO: Why doesn't this work?
set(CMAKE_EXECUTABLE_SUFFIX .elf)

################################################################################
### Flags
################################################################################
# Create standard flags
set(ASFLAGS "-m${MODEL} -xassembler-with-cpp")
set(CFLAGS_NO_MODEL "-Wall -m32bit-doubles")
set(CFLAGS "${CFLAGS} -m${MODEL} ${CFLAGS_NO_MODEL}")
set(CSTANDARD "-std=c99")
set(CXXFLAGS "${CXXFLAGS} ${CFLAGS} -fno-threadsafe-statics -fno-rtti")
set(CXXSTANDARD "-std=gnu++0x")

# Set flags
set(CMAKE_ASM_FLAGS ${ASFLAGS})
set(CMAKE_C_FLAGS "${CFLAGS} ${CSTANDARD}")
set(CMAKE_CXX_FLAGS "${CXXFLAGS} ${CXXSTANDARD}")
set(CMAKE_AR_FLAGS "cr")
set(LDFLAGS "-Xlinker -Map=main.rawmap")

# Add library search paths
set(CMAKE_EXE_LINKER_FLAGS "${LDFLAGS}")

################################################################################
### Compilers
################################################################################
set(CMAKE_C_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_CXX_COMPILE_OBJECT
"<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")

set(CMAKE_ASM_COMPILE_OBJECT
"<CMAKE_ASM_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")

################################################################################
### Linkers
################################################################################
set(CMAKE_C_LINK_EXECUTABLE
"<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o<TARGET> <OBJECTS> <LINK_LIBRARIES>")

include_directories(${PROPWARE_PATH})
