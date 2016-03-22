set(MODEL xmm-single)

set(TARGET PropWare_${MODEL})
add_library(${TARGET} STATIC ${PROPWARE_SOURCES})
set_compile_flags(${TARGET})
include_directories(${PROJECT_SOURCE_DIR})
set_linker(${TARGET})
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME PropWare)
install(TARGETS ${TARGET}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
