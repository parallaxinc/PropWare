cmake_minimum_required (VERSION 3.0.0)

set(MODEL xmmc)
set_compile_flags()

add_library(Libpropeller_${MODEL} STATIC ${LIBPROPELLER_OBJECTS})
install(TARGETS Libpropeller_${MODEL}
    DESTINATION PropWare/lib
    COMPONENT propware
    EXPORT PropWare-targets)

