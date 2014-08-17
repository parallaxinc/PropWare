set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Propeller)

if (NOT DEFINED PROPGCC_PREFIX)
    set(PROPGCC_PREFIX $ENV{PROPGCC_PREFIX})
    if (NOT PROPGCC_PREFIX)
        message(FATAL_ERROR "Please define 'PROPGCC_PREFIX' either as an environment variable or CMake variable. The value should be the installation directory for PropGCC")
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

# specify the cross compiler
file(TO_CMAKE_PATH "${PROPGCC_PREFIX}/bin" GCC_PATH)
set(CMAKE_C_COMPILER   ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_CXX_COMPILER ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_ASM_COMPILER ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_RANLIB ${GCC_PATH}/propeller-elf-ranlib)
set(CMAKE_OBJCOPY ${GCC_PATH}/propeller-elf-objcopy)
set(CMAKE_OBJDUMP ${GCC_PATH}/propeller-elf-objdump)
set(CMAKE_ELF_LOADER ${GCC_PATH}/propeller-load)

set(CMAKE_COGC_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_COGCXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_ECOGC_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ECOGCXX_COMPILER ${CMAKE_CXX_COMPILER})

set(CMAKE_FIND_ROOT_PATH ${PROPGCC_PREFIX} ${PROPWARE_PATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)
