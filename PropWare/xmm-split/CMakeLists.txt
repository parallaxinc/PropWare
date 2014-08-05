cmake_minimum_required (VERSION 3.0.0)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL xmm-split)
include(${PROPWARE_PATH}/SetPropWareFlags.cmake)

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
