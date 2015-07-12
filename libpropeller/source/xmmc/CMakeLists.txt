cmake_minimum_required (VERSION 3.0.0)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL xmmc)

include(${PROJECT_SOURCE_DIR}/CMakePropellerSetFlags.cmake)
add_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})

