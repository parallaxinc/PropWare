cmake_minimum_required (VERSION 3.0.0)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
    file(TO_NATIVE_PATH PROPWARE_PATH ${PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)
set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )

project(I2C_Demo)

add_executable(${PROJECT_NAME} ${PROJECT_NAME})
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)
