set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Propeller)

if (WIN32)
    set(GCC_SUFFIX .exe)
endif ()

if (NOT DEFINED PROPGCC_PREFIX)
    set(PROPGCC_PREFIX $ENV{PROPGCC_PREFIX})
    if (NOT PROPGCC_PREFIX)
        message(FATAL_ERROR "Please define 'PROPGCC_PREFIX' either as an environment variable or CMake variable. The value should be the installation directory for PropGCC")
    endif ()
endif ()
file(TO_CMAKE_PATH "${PROPGCC_PREFIX}/bin" GCC_PATH)

# specify the cross compiler
set(CMAKE_C_COMPILER   ${GCC_PATH}/propeller-elf-gcc${GCC_SUFFIX} CACHE FILEPATH "C Compiler")
set(CMAKE_CXX_COMPILER ${GCC_PATH}/propeller-elf-gcc${GCC_SUFFIX} CACHE FILEPATH "C++ Compiler")
set(CMAKE_ASM_COMPILER ${GCC_PATH}/propeller-elf-gcc${GCC_SUFFIX} CACHE FILEPATH "Assembly compiler")
set(CMAKE_RANLIB       ${GCC_PATH}/propeller-elf-ranlib${GCC_SUFFIX} CACHE FILEPATH "Ranlib")
set(CMAKE_OBJCOPY      ${GCC_PATH}/propeller-elf-objcopy${GCC_SUFFIX} CACHE FILEPATH "Object copy")
set(CMAKE_OBJDUMP      ${GCC_PATH}/propeller-elf-objdump${GCC_SUFFIX} CACHE FILEPATH "Object dump")
set(CMAKE_AR           ${GCC_PATH}/propeller-elf-ar${GCC_SUFFIX} CACHE FILEPATH "Archiver")
set(CMAKE_ELF_LOADER   ${GCC_PATH}/propeller-load${GCC_SUFFIX} CACHE FILEPATH "Elf loader")
set(CMAKE_GDB          ${GCC_PATH}/propeller-elf-gdb${GCC_SUFFIX} CACHE FILEPATH "GNU DeBugger (GDB)")

set(CMAKE_COGC_COMPILER    ${CMAKE_C_COMPILER})
set(CMAKE_COGCXX_COMPILER  ${CMAKE_CXX_COMPILER})
set(CMAKE_ECOGC_COMPILER   ${CMAKE_C_COMPILER})
set(CMAKE_ECOGCXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_DAT_COMPILER     ${CMAKE_C_COMPILER})

set(CMAKE_FIND_ROOT_PATH ${PROPGCC_PREFIX})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)
