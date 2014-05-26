cmake_minimum_required (VERSION 2.6.4)

set(MODEL lmm)
project(PropWare_${MODEL})
set(CMAKE_PROJECT_NAME PropWare_${MODEL})

add_library(${CMAKE_PROJECT_NAME} STATIC
        ../hd44780
        ../l3g
        ../max6675
        ../mcp3000
        ../pin
        ../port
        ../PropWare
        ../sd
        ../spi
        ../spi_as.S
        ../uart)

set(BOARD QUICKSTART)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/PropellerToolchain.cmake)
