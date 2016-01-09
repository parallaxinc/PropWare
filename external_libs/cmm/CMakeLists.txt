cmake_minimum_required (VERSION 3.0.0)

set(MODEL cmm)
set_compile_flags()

add_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
add_library(Simple_${MODEL} STATIC ${SIMPLE_OBJECTS})
set_linker(Libpropeller_${MODEL})
set_linker(Simple_${MODEL})
install(TARGETS Libpropeller_${MODEL} Simple_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware
    EXPORT PropWare-targets)
