################################################################################
### Finalize Compile Flags
################################################################################

# Check if the deprecated variable name is set
if (DEFINED CFLAGS OR DEFINED CXXFLAGS)
    message(WARN ": CFLAGS and CXXFLAGS variable names have been replaced by C_FLAGS and CXX_FLAGS.")
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
