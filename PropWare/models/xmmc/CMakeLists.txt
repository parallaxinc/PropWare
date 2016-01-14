set(MODEL xmmc)
set_compile_flags()

add_library(PropWare_${MODEL} STATIC ${PROPWARE_SOURCES})
set_target_properties(PropWare_${MODEL} PROPERTIES OUTPUT_NAME PropWare)
install(TARGETS PropWare_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
