set(BOARD dna)
set(MODEL cmm)

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
create_test(spi_test                spi_test)

set_tests_properties(
    sample_test
    stringbuilder_test
    queue_test
    utility_test
    PROPERTIES LABELS hardware-independent)

install(FILES PropWareTests.h
    DESTINATION PropWare/include/PropWare
    COMPONENT propware)
