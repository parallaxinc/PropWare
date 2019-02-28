set(BOARD dna)
set(MODEL cmm)

create_test(eeprom_test             eeprom_test.cpp)
create_test(fatfilereader_test      fatfilereader_test.cpp)
create_test(fatfilewriter_test      fatfilewriter_test.cpp)
create_test(fatfs_test              fatfs_test.cpp)
create_test(i2c_test                i2c_test.cpp)
create_test(pin_test                pin_test.cpp)
create_test(ping_test               ping_test.cpp)
create_test(queue_test              queue_test.cpp)
create_test(sample_test             sample_test.cpp)
create_test(scanner_test            scanner_test.cpp)
create_test(sd_test                 sd_test.cpp)
create_test(spi_test                spi_test.cpp)
create_test(stepper_test            stepper_test.cpp)
create_test(stringbuilder_test      stringbuilder_test.cpp)
create_test(utility_test            utility_test.cpp)

set_tests_properties(
    eeprom_test
    i2c_test
    ping_test
    queue_test
    sample_test
    scanner_test
    stepper_test
    stringbuilder_test
    utility_test
    PROPERTIES LABELS hardware-independent)

install(FILES PropWareTests.h
    DESTINATION share/PropWare/include/PropWare/PropWare
    COMPONENT propware)
