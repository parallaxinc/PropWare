set(MODEL xmm-split)

# WAV player need to be optimized for speed, not size
set_source_files_properties("${PROJECT_SOURCE_DIR}/propsideworkspace/Learn/Simple Libraries/Audio/libwavplayer/wavplayer.c"
    PROPERTIES COMPILE_FLAGS "-O3")

create_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
create_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
set_target_properties(Libpropeller_${MODEL} PROPERTIES OUTPUT_NAME Libpropeller)
set_target_properties(Simple_${MODEL} PROPERTIES OUTPUT_NAME Simple)
install(TARGETS Libpropeller_${MODEL} Simple_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
