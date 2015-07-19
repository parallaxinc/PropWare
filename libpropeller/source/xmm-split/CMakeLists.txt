cmake_minimum_required (VERSION 3.0.0)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL xmm-split)
set_compile_flags()

add_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
install(TARGETS Libpropeller_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware)

