set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Propeller)

if (NOT DEFINED PROPGCC_PREFIX)
    set(PROPGCC_PREFIX $ENV{PROPGCC_PREFIX})
    if (NOT PROPGCC_PREFIX)
        message(FATAL_ERROR "Please define 'PROPGCC_PREFIX' either as an environment variable or CMake variable. The value should be the location of PropGCC. For example, on Linux or Mac the default is /opt/parallax.")
    endif (NOT PROPGCC_PREFIX)
endif (NOT DEFINED PROPGCC_PREFIX)

# PROPWARE_PATH must be defined here so that the header files are included in
# the search path
if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
    if (NOT PROPWARE_PATH)
        message(FATAL_ERROR "Please define 'PROPWARE_PATH' either as an environment variable or CMake variable. The value should be the location of the PropWare root directory.")
    endif (NOT PROPWARE_PATH)
endif (NOT DEFINED PROPWARE_PATH)

if (NOT DEFINED MODEL)
    set(MODEL lmm)
endif (NOT DEFINED MODEL)

# XMM model is retroactively renamed xmm-split
if (${MODEL} STREQUAL xmm)
    set(MODEL xmm-split)
endif (${MODEL} STREQUAL xmm)

# specify the cross compiler
set(GCC_PATH ${PROPGCC_PREFIX}/bin)
set(CMAKE_C_COMPILER   ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_CXX_COMPILER ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_ASM_COMPILER ${GCC_PATH}/propeller-elf-gcc)
#set(CMAKE_AR ${GCC_PATH}/propeller-elf-ar)  # For some reason, defining this breaks it all
set(CMAKE_RANLIB ${GCC_PATH}/propeller-elf-ranlib)
set(CMAKE_ELF_LOADER ${GCC_PATH}/propeller-load)

set(CMAKE_FIND_ROOT_PATH ${PROPGCC_PREFIX} ${PROPWARE_PATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)