cmake_minimum_required (VERSION 3.0.0)

find_package(PropWare REQUIRED)

set(BOARD dna)
set(MODEL cmm)
set(COMMON_FLAGS "-Os")

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
