set(can_use_assembler TRUE)

SET(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_BUILD_TYPE None)

# specify the cross compiler
set(GCC_PATH ${PROPGCC_PREFIX}/bin)
set(CMAKE_ASM_COMPILER   ${GCC_PATH}/propeller-elf-as)
set(CMAKE_C_COMPILER   ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_CXX_COMPILER ${GCC_PATH}/propeller-elf-gcc)
set(CMAKE_AR ${GCC_PATH}/propeller-elf-ar)

set(CMAKE_FIND_ROOT_PATH ${PROPGCC_PREFIX} ${PROPWARE_PATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM never)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY only)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE only)
