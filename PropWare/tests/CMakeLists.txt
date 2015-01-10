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

project(propware_tests)

create_executable(sample_test           sample_test)
create_executable(pin_test              pin_test)
create_executable(fatfs_test            fatfs_test)
create_executable(uart_simplex_test     uart_simplex_test)
create_executable(sd_test               sd_test)
create_executable(queue_test            queue_test)
