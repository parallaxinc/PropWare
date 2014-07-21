cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(CFLAGS "-Os")
set(CXXFLAGS )
set(MODEL xmm-single)
include(${PROPWARE_PATH}/CMakeModules/Generic-gcc-Propeller.cmake)

project(PropWare_${MODEL})

add_library(${PROJECT_NAME} STATIC
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
