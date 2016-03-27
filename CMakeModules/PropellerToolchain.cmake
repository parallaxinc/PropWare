set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Propeller)

if (NOT DEFINED GCC_PATH)
    if (WIN32)
        set(EXE_SUFFIX ".exe")
    endif ()

    # Give priority to explicit defnitions and make sure the PATH variable is checked _last_ by excluding the default
    # search mechanism and then manually adding it into the end
    find_path(GCC_PATH
        NAMES
        propeller-elf-gcc${EXE_SUFFIX}
        PATHS
        "$ENV{GCC_PATH}"
        "${PROPGCC_PREFIX}/bin"
        "$ENV{PROPGCC_PREFIX}/bin"
        ENV PATH
        NO_SYSTEM_ENVIRONMENT_PATH)

    file(TO_CMAKE_PATH "/opt/parallax/bin" DEFAULT_LINUX_PATH_1)
    file(TO_CMAKE_PATH "C:\\PropGCC\\bin" DEFAULT_WINDOWS_PATH_1)
    file(TO_CMAKE_PATH "C:\\parallax\\bin" DEFAULT_WINDOWS_PATH_2)
    file(TO_CMAKE_PATH "C:\\Program Files\\SimpleIDE\\propeller-gcc\\bin" DEFAULT_WINDOWS_PATH_3)
    file(TO_CMAKE_PATH "C:\\Program Files (x86)\\SimpleIDE\\propeller-gcc\\bin" DEFAULT_WINDOWS_PATH_4)

    if (NOT GCC_PATH)
        find_path(GCC_PATH
            NAMES
            propeller-elf-gcc${EXE_SUFFIX}
            PATHS
            "${DEFAULT_LINUX_PATH_1}"
            "${DEFAULT_WINDOWS_PATH_1}"
            "${DEFAULT_WINDOWS_PATH_2}"
            "${DEFAULT_WINDOWS_PATH_3}"
            "${DEFAULT_WINDOWS_PATH_4}")
    endif ()
endif ()

# specify the cross compiler
find_program(CMAKE_C_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_CXX_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_ASM_COMPILER propeller-elf-gcc
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_RANLIB propeller-elf-ranlib
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_OBJCOPY propeller-elf-objcopy
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_OBJDUMP propeller-elf-objdump
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_AR propeller-elf-ar
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_ELF_LOADER propeller-load
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_GDB propeller-elf-gdb
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_ELF_SIZE propeller-elf-size
    PATHS "${GCC_PATH}"
    NO_SYSTEM_ENVIRONMENT_PATH)
find_program(CMAKE_SPIN2DAT_COMPILER spin2cpp
    PATHS "${GCC_PATH}")

find_program(ELF_SIZER elfsizer
    PATHS "${GCC_PATH}")

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
