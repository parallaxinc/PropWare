################################################################################
### Template code. Do not modify                                               #
                                                                               #
cmake_minimum_required (VERSION 3.0.0)                                         #
# Aside from cmake_minimum_required, this must be the first two lines          #
# of the file                                                                  #
file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                          #
include(${PROPWARE_PATH}/CMakePropellerHeader.cmake)                           #
################################################################################

set(BOARD QUICKSTART)
set(MODEL lmm)
set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )

# This demo does not rely on any non-PropWare classes. It is therefore compatible with the `WHOLE_PROGRAM` option, which
# allows PropGCC to perform more agressive optimizations
set(WHOLE_PROGRAM ON)

project(FatFS_Demo)

create_simple_executable(${PROJECT_NAME} ${PROJECT_NAME})
