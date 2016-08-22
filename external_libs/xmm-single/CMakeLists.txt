set(MODEL xmm-single)

# WAV player need to be optimized for speed, not size
set_source_files_properties("${SIMPLE_ROOT}/Audio/libwavplayer/wavplayer.c"
    PROPERTIES COMPILE_FLAGS "-O3")

create_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
create_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
create_library(LibPropelleruino_${MODEL} STATIC ${LIBPROPELLERUINO_OBJECTS})
set_target_properties(Libpropeller_${MODEL} PROPERTIES OUTPUT_NAME Libpropeller)
set_target_properties(Simple_${MODEL} PROPERTIES OUTPUT_NAME Simple)
set_target_properties(LibPropelleruino_${MODEL} PROPERTIES OUTPUT_NAME LibPropelleruino)
install(TARGETS Libpropeller_${MODEL} Simple_${MODEL} LibPropelleruino_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)

