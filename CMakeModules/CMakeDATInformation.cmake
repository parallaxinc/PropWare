# This file sets the basic flags for the COGC language in CMake.
# It also loads the available platform file for the system-compiler
# if it exists.
# It also loads a system - compiler - processor (or target hardware)
# specific file, which is mainly useful for crosscompiling and embedded systems.

# some compilers use different extensions (e.g. sdcc uses .rel)
# so set the extension here first so it can be overridden by the compiler specific file
set(CMAKE_DAT_OUTPUT_EXTENSION .o)

set(_INCLUDED_FILE 0)

set(CMAKE_BASE_NAME gcc)

set (CMAKE_DAT_FLAGS "" CACHE STRING
     "")

set(CMAKE_INCLUDE_FLAG_DAT ${CMAKE_INCLUDE_FLAG_C})

include(CMakeCommonLanguageInclude)

set(CMAKE_DAT_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_DAT_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_DAT_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})

set(CMAKE_DAT_COMPILE_OBJECT
"${CMAKE_COMMAND} -DOBJCOPY=\"${CMAKE_OBJCOPY}\" -DSOURCE=<SOURCE> -DOUTPUT=<OBJECT> -P ${PropWare_DAT_SYMBOL_CONVERTER}"
)

mark_as_advanced(
    CMAKE_DAT_COMPILER
    CMAKE_DAT_FLAGS
)

set(CMAKE_DAT_INFORMATION_LOADED 1)
