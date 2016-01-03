cmake_minimum_required (VERSION 3.0.0)

set(MODEL xmmc)
set_compile_flags()

add_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
set_linker(Simple_${MODEL})
install(TARGETS Simple_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware
    EXPORT PropWare-targets)
