cmake_minimum_required (VERSION 2.6.4)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
endif (NOT DEFINED PROPWARE_PATH)

set(CFLAGS "-Os")
set(CXXFLAGS )
set(MODEL xmm-single)
include(${PROPWARE_PATH}/CMakeModules/Generic-gcc-Propeller.cmake)

project(Libpropeller_${MODEL})

include(../libpropellerObjects.cmake)
add_library(${PROJECT_NAME} STATIC ${LIBPROPELLER_OBJECTS})
