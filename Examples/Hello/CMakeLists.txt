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

set(PROPWARE_PRINT_FLOAT OFF)
set(LINK_TINY ON)

project(Hello_Demo C CXX DAT)

add_executable(${PROJECT_NAME}
    ${PROJECT_NAME}
    pst.dat)

################################################################################
### Template code. Do not modify                                               #
                                                                               #
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)                           #
################################################################################
