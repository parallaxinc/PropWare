set(MODEL cog)

create_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
create_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
set_target_properties(Libpropeller_${MODEL} PROPERTIES OUTPUT_NAME Libpropeller)
set_target_properties(Simple_${MODEL} PROPERTIES OUTPUT_NAME Simple)
install(TARGETS Libpropeller_${MODEL} Simple_${MODEL}
    DESTINATION PropWare/lib/${MODEL}
    COMPONENT propware
    EXPORT PropWare-targets)
