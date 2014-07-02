cmake_minimum_required (VERSION 2.6.4)

set(MODEL xmm-single)
project(Libpropeller_${MODEL})
set(CMAKE_PROJECT_NAME Libpropeller_${MODEL})

include(../libpropellerObjects.cmake)

add_library(${CMAKE_PROJECT_NAME} STATIC ${LIBPROPELLER_OBJECTS})

set(BOARD QUICKSTART)
set(CFLAGS "-Os")

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

include(${PROPWARE_PATH}/PropellerToolchain.cmake)
