cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(CFLAGS "-Os")
set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)

project(SD_Demo)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)
