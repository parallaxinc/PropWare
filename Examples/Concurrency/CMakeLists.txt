cmake_minimum_required (VERSION 2.6.4)

project(Concurrency_Demo)
set(CMAKE_PROJECT_NAME Concurrency_Demo)

add_executable(${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.cpp)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/CMake_header.cmake)

target_link_libraries(${CMAKE_PROJECT_NAME}.elf ${PROPWARE_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME}.elf ${SIMPLE_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME}.elf ${LIBPROPELLER_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME}.elf tiny)