cmake_minimum_required (VERSION 3.0.0)

if (NOT DEFINED PROPWARE_PATH)
    file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)
endif ()

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL cmm)
include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

project(PropWare_${MODEL})

add_library(${PROJECT_NAME} STATIC
        ../hd44780
        ../l3g
        ../max6675
        ../mcp3000
        ../pin
        ../port
        ../printcapable
        ../printer
        ../PropWare
        ../scancapable
        ../scanner
        ../sd
        ../seeedtft
        ../seeedtftfast
        ../seeedtftfast_as.S
        ../spi
        ../spi_as.S
        ../uart/abstractduplexuart
        ../uart/abstractsimplexuart
        ../uart/duplexuart
        ../uart/fullduplexuart
        ../uart/halfduplexuart
        ../uart/simplexuart
        ../uart/sharedsimplexuart
        ../uart/uart)
