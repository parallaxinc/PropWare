set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Propeller)

if (NOT DEFINED GCC_PATH)
    file(TO_CMAKE_PATH "/opt/parallax/bin" DEFAULT_LINUX_PATH_1)
    file(TO_CMAKE_PATH "C:\\Program Files\\SimpleIDE\\propeller-gcc\\bin" DEFAULT_WINDOWS_PATH_1)
    file(TO_CMAKE_PATH "C:\\Program Files (x86)\\SimpleIDE\\propeller-gcc\\bin" DEFAULT_WINDOWS_PATH_2)
    file(TO_CMAKE_PATH "C:\\PropGCC\\bin" DEFAULT_WINDOWS_PATH_3)

    find_path(GCC_PATH
        NAMES
            propeller-elf-gcc
            propeller-elf-gcc.exe
        PATHS
            "$ENV{PROPGCC_PREFIX}/bin"
            "${PROPGCC_PREFIX}/bin"
            "$ENV{GCC_PATH}"
            "${DEFAULT_LINUX_PATH_1}"
            "${DEFAULT_WINDOWS_PATH_1}"
            "${DEFAULT_WINDOWS_PATH_2}"
            "${DEFAULT_WINDOWS_PATH_3}")
endif ()

# specify the cross compiler
find_program(CMAKE_C_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}" ENV PATH)
if (NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "Can't find PropGCC")
endif ()

find_program(CMAKE_CXX_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_ASM_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_RANLIB propeller-elf-ranlib
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_OBJCOPY propeller-elf-objcopy
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_OBJDUMP propeller-elf-objdump
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_AR propeller-elf-ar
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_ELF_LOADER propeller-load
    PATHS "${GCC_PATH}" ENV PATH)
find_program(CMAKE_GDB propeller-elf-gdb
    PATHS "${GCC_PATH}" ENV PATH)

set(CMAKE_COGC_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_COGCXX_COMPILER "${CMAKE_CXX_COMPILER}")
set(CMAKE_ECOGC_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_ECOGCXX_COMPILER "${CMAKE_CXX_COMPILER}")
set(CMAKE_DAT_COMPILER "${CMAKE_C_COMPILER}")

get_filename_component(GCC_PATH "${CMAKE_C_COMPILER}" DIRECTORY CACHE)

set(CMAKE_FIND_ROOT_PATH "${PROPGCC_PREFIX}/..")

# Don't search for programs compiled for the Propeller architecture
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# Only find libraries and include directories built for the Propeller architecture
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)
