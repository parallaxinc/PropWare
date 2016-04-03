set(MODEL xmm-split)

set(TARGET PropWare_${MODEL})
create_library(${TARGET} ${PROPWARE_SOURCES})
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME PropWare)
install(TARGETS ${TARGET}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
