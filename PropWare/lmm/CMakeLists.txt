cmake_minimum_required (VERSION 3.0.0)

set(MODEL lmm)
set_compile_flags()

add_library(PropWare_${MODEL} STATIC ${PROPWARE_SOURCES})
install(TARGETS PropWare_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware)
