set(MODEL xmm-split)

add_library(PropWare_${MODEL} STATIC ${PROPWARE_SOURCES})
set_compile_flags()
include_directories(${PROJECT_SOURCE_DIR})
set_linker(PropWare_${MODEL})
set_target_properties(PropWare_${MODEL} PROPERTIES OUTPUT_NAME PropWare)
install(TARGETS PropWare_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
