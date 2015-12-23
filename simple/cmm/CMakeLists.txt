cmake_minimum_required (VERSION 3.0.0)

set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )
set(MODEL cmm)
set_compile_flags()

add_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
install(TARGETS Simple_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware
    EXPORT PropWare-targets)
