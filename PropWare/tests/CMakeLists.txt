###################################################################################
### Template code. Do not modify                                                  #
                                                                                  #
cmake_minimum_required (VERSION 3.0.0)                                            #
# Aside from cmake_minimum_required, this must be the first two lines of the file #
file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                             #
include(${PROPWARE_PATH}/CMakePropellerHeader.cmake)                              #
###################################################################################

set(BOARD dna)
set(MODEL cmm)
set(COMMON_FLAGS "-Os")

project(ProWare_Tests)

create_executable(sample_test           sample_test)
create_executable(pin_test              pin_test)
create_executable(fatfilereader_test    fatfilereader_test)
create_executable(fatfilewriter_test    fatfilewriter_test)
create_executable(fatfs_test            fatfs_test)
create_executable(uart_simplex_test     uart_simplex_test)
create_executable(sd_test               sd_test)
create_executable(stringbuilder_test    stringbuilder_test)
create_executable(queue_test            queue_test)
create_executable(utility_test          utility_test)
