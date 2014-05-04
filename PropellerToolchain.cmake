if (NOT DEFINED PROPGCC_PREFIX)
    set(PROPGCC_PREFIX $ENV{PROPGCC_PREFIX})
    if (NOT PROPGCC_PREFIX)
        message(FATAL_ERROR "Please define 'PROPGCC_PREFIX' either as an environment variable or CMake variable. The value should be the location of PropGCC. For example, on Linux or Mac the default is /opt/parallax.")
    endif (NOT PROPGCC_PREFIX)
endif (NOT DEFINED PROPGCC_PREFIX)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
    if (NOT PROPWARE_PATH)
        message(FATAL_ERROR "Please define 'PROPWARE_PATH' either as an environment variable or CMake variable. The value should be the location of the PropWare root directory.")
    endif (NOT PROPWARE_PATH)
endif (NOT DEFINED PROPWARE_PATH)

if (NOT DEFINED MODEL)
    set(MODEL lmm)
endif (NOT DEFINED MODEL)

if (${MODEL} MATCHES xmm)
    set(MODEL xmm-split)
endif (${MODEL} MATCHES xmm)

set(CAN_USE_ASSEMBLER TRUE)
enable_language(ASM)

SET(CMAKE_SYSTEM_NAME Propeller)

# specify the cross compiler
set(GCC_PATH ${PROPGCC_PREFIX}/bin)
set(CMAKE_ASM_COMPILER   ${GCC_PATH}/propeller-elf-gcccd )
set(CMAKE_C_COMPILER   ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_CXX_COMPILER ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_AR ${GCC_PATH}/propeller-elf-ar)
set(CMAKE_RANLIB ${GCC_PATH}/propeller-elf-ranlib)

set(CMAKE_FIND_ROOT_PATH ${PROPGCC_PREFIX} ${PROPWARE_PATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)

# Add standard flags
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
set(LDFLAGS "-Xlinker -Map=main.rawmap")

include_directories(${PROPWARE_PATH})
