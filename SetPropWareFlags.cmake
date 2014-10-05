################################################################################
### Finalize Compile Flags
################################################################################

# Handle user options
if (32_BIT_DOUBLES)
    set(C_FLAGS "${C_FLAGS} -m32bit-doubles")
    set(CXX_FLAGS "${CXX_FLAGS} -m32bit-doubles")
endif ()

if (WARN_ALL)
    set(C_FLAGS "${C_FLAGS} -Wall")
    set(CXX_FLAGS "${CXX_FLAGS} -Wall")
endif ()

if (AUTO_C_STD)
    set(C_FLAGS "${C_FLAGS} -std=c99")
endif ()

if (AUTO_CXX_STD)
    set(CXX_FLAGS "${CXX_FLAGS} -std=gnu++0x")
endif ()

if (AUTO_CUT_SECTIONS)
    set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --gc-sections")
endif ()

if (PROPWARE_PRINT_FLOAT)
    add_definitions(-DENABLE_PROPWARE_PRINT_FLOAT)
endif ()

# Check if a deprecated variable name is set
if (DEFINED CFLAGS OR DEFINED CXXFLAGS)
    message(WARN ": The variables `CFLAGS` and `CXXFLAGS` have been replaced by `C_FLAGS` and `CXX_FLAGS`.")
    set(C_FLAGS ${CFLAGS})
    set(CXX_FLAGS ${CXXFLAGS})
    set(CFLAGS )
    set(CXXFLAGS )
endif()

# Overwite the old flags
set(CMAKE_ASM_FLAGS     "${COMMON_FLAGS}    ${ASM_FLAGS}")
set(CMAKE_C_FLAGS       "${COMMON_FLAGS}    ${C_FLAGS}")
set(CMAKE_CXX_FLAGS     "${COMMON_FLAGS}    ${CXX_FLAGS}")

set(CMAKE_COGC_FLAGS    "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${COGC_FLAGS}")
set(CMAKE_COGCXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${COGCXX_FLAGS}")
set(CMAKE_ECOGC_FLAGS   "${CMAKE_C_FLAGS}   ${COMMON_COG_FLAGS} ${ECOGC_FLAGS}")
set(CMAKE_ECOGCXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COG_FLAGS} ${ECOGCXX_FLAGS}")

# If no model is specified, we must choose a default so that the proper libraries can be linked
if (NOT DEFINED MODEL)
    set(MODEL lmm)
endif ()

# XMM model is retroactively renamed xmm-split
if (${MODEL} STREQUAL xmm)
    set(MODEL xmm-split)
endif ()

set(CMAKE_ASM_FLAGS     "${CMAKE_ASM_FLAGS} -m${MODEL}")
set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}   -m${MODEL}")
set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -m${MODEL}")
set(CMAKE_DAT_FLAGS     "${MODEL}")
