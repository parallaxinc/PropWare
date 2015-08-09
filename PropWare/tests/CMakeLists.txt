cmake_minimum_required (VERSION 3.0.0)

find_package(PropWare REQUIRED)

set(BOARD dna)
set(MODEL cmm)
set(COMMON_FLAGS "-Os")

create_test(sample_test             sample_test)
create_test(pin_test                pin_test)
create_test(fatfilereader_test      fatfilereader_test)
create_test(fatfilewriter_test      fatfilewriter_test)
create_test(fatfs_test              fatfs_test)
create_test(uart_simplex_test       uart_simplex_test)
create_test(sd_test                 sd_test)
create_test(stringbuilder_test      stringbuilder_test)
create_test(queue_test              queue_test)
create_test(utility_test            utility_test)
