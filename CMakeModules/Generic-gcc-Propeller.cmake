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

#set(CMAKE_C_FLAGS_INIT "$ENV{CFLAGS} ${CMAKE_C_FLAGS_INIT} -Wall -m32bit-doubles")
## avoid just having a space as the initial value for the cache
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT}" CACHE STRING
#     "Flags used by the compiler during all build types.")

# Create standard flags
set(ASMFLAGS "-xassembler-with-cpp")
set(CFLAGS "${CFLAGS} -Wall -m32bit-doubles")
set(CSTANDARD "-std=c99")
set(CXXFLAGS "${CXXFLAGS} -fno-threadsafe-statics -fno-rtti")
set(CXXSTANDARD "-std=gnu++0x")

# Set flags
set(CMAKE_ASM_FLAGS "${ASMFLAGS} -m${MODEL}")
set(CMAKE_C_FLAGS "${CFLAGS} ${CSTANDARD} -m${MODEL} ")
set(CMAKE_CXX_FLAGS "${CFLAGS} ${CXXFLAGS} ${CXXSTANDARD} -m${MODEL}")
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
set(CMAKE_C_ARCHIVE_CREATE
"<CMAKE_AR> cr <TARGET> <LINK_FLAGS> <OBJECTS>")

set(CMAKE_C_ARCHIVE_APPEND
"<CMAKE_AR> r  <TARGET> <LINK_FLAGS> <OBJECTS>")

set(CMAKE_C_ARCHIVE_FINISH
"<CMAKE_RANLIB> <TARGET>")

set(CMAKE_C_LINK_EXECUTABLE
"<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o<TARGET> <OBJECTS> <LINK_LIBRARIES>")

include_directories(${PROPWARE_PATH})

################################################################################
### Custom Propeller "Languages"
###
### NOTE: Set src file ext in CMake<LANG>Compiler.cmake.in
################################################################################
#set(CMAKE_SPIN_OUTPUT_EXTENSION .o)
#set(CMAKE_DAT_SOURCE_FILE_EXTENSIONS _firmware.o)

#############
# COGC
set(CMAKE_COGC_OUTPUT_EXTENSION .cog)

set(CMAKE_COGC_FLAGS "${CFLAGS} ${CSTANDARD} -mcog -xc -r")
set(CMAKE_INCLUDE_FLAG_COGC ${CMAKE_INCLUDE_FLAG_C})

set(CMAKE_COGC_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_COGC_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_COGC_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})
set(CMAKE_COGC_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>"
"${CMAKE_OBJCOPY} --localize-text --rename-section .text=<OBJECT> <OBJECT>")

############
# COGCXX
set(CMAKE_COGCXX_OUTPUT_EXTENSION .cogcxx)

set(CMAKE_COGCXX_FLAGS "${CFLAGS} ${CXXFLAGS} ${CXXSTANDARD} -mcog -xc++ -r")
set(CMAKE_INCLUDE_FLAG_COGCXX ${CMAKE_INCLUDE_FLAG_C})

set(CMAKE_COGC_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_COGC_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_COGC_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})
set(CMAKE_COGC_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>"
"${CMAKE_OBJCOPY} --localize-text --rename-section .text=<OBJECT> <OBJECT>")

#############
# ECOGC
set(CMAKE_ECOGC_OUTPUT_EXTENSION .ecog)

set(CMAKE_ECOGC_FLAGS "${CFLAGS} ${CSTANDARD} -mcog -xc -r")
set(CMAKE_INCLUDE_FLAG_ECOGC ${CMAKE_INCLUDE_FLAG_C})

set(CMAKE_ECOGC_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_ECOGC_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_ECOGC_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})
set(CMAKE_ECOGC_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>"
"${CMAKE_OBJCOPY} --localize-text --rename-section .text=<OBJECT> <OBJECT>")

############
# ECOGCXX
set(CMAKE_ECOGCXX_OUTPUT_EXTENSION .ecogcxx)

set(CMAKE_ECOGCXX_FLAGS "${CFLAGS} ${CXXFLAGS} ${CXXSTANDARD} -mcog -xc++ -r")
set(CMAKE_INCLUDE_FLAG_ECOGCXX ${CMAKE_INCLUDE_FLAG_C})

set(CMAKE_ECOGC_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_ECOGC_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_ECOGC_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})
set(CMAKE_ECOGC_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>"
"${CMAKE_OBJCOPY} --localize-text --rename-section .text=<OBJECT> <OBJECT>")
