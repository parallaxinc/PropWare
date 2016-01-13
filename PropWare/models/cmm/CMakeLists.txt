cmake_minimum_required (VERSION 3.0.0)

set(MODEL cmm)
set_compile_flags()

add_library(PropWare_${MODEL} STATIC ${PROPWARE_SOURCES})
set_target_properties(PropWare_${MODEL} PROPERTIES OUTPUT_NAME PropWare)
install(TARGETS PropWare_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
