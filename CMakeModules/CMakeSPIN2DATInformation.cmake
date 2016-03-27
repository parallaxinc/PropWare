# This file sets the basic flags for the COGC language in CMake.
# It also loads the available platform file for the system-compiler
# if it exists.
# It also loads a system - compiler - processor (or target hardware)
# specific file, which is mainly useful for crosscompiling and embedded systems.

# some compilers use different extensions (e.g. sdcc uses .rel)
# so set the extension here first so it can be overridden by the compiler specific file
set(CMAKE_SPIN2DAT_OUTPUT_EXTENSION .dat)

set(_INCLUDED_FILE 0)

set(CMAKE_BASE_NAME spin2cpp)

set (CMAKE_SPIN2DAT_FLAGS "" CACHE STRING
     "")

set(CMAKE_INCLUDE_FLAG_SPIN2DAT "")

include(CMakeCommonLanguageInclude)

set(CMAKE_SPIN2DAT_ARCHIVE_CREATE "${CMAKE_C_ARCHIVE_CREATE}")
set(CMAKE_SPIN2DAT_ARCHIVE_APPEND "${CMAKE_C_ARCHIVE_APPEND}")
set(CMAKE_SPIN2DAT_ARCHIVE_FINISH "${CMAKE_C_ARCHIVE_FINISH}")

set(CMAKE_SPIN2DAT_COMPILE_OBJECT
    "${CMAKE_SPIN2DAT_COMPILER} --dat -o <OBJECT> <SOURCE>"
    "${CMAKE_COMMAND} -DOBJCOPY=\"${CMAKE_OBJCOPY}\" -DSTRIP=.spin -DSOURCE=<OBJECT> -DOUTPUT=<OBJECT> -P ${PropWare_DAT_SYMBOL_CONVERTER}"
)

mark_as_advanced(
    CMAKE_SPIN2DAT_COMPILER
    CMAKE_SPIN2DAT_FLAGS
)

set(CMAKE_SPIN2DAT_INFORMATION_LOADED 1)
