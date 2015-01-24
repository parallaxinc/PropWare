################################################################################
### Finalize Compile Flags
################################################################################

include_directories(${PROPWARE_PATH})

# Handle user options
if (32_BIT_DOUBLES)
    if (${32_BIT_DOUBLES_SET})
    else ()
        set(32_BIT_DOUBLES_SET 1)
        set(C_FLAGS "${C_FLAGS} -m32bit-doubles")
        set(CXX_FLAGS "${CXX_FLAGS} -m32bit-doubles")
    endif ()
endif ()

if (WARN_ALL)
    if (${WARN_ALL_SET})
    else ()
        set(WARN_ALL_SET 1)
        set(C_FLAGS "${C_FLAGS} -Wall")
        set(CXX_FLAGS "${CXX_FLAGS} -Wall")
    endif ()
endif ()

if (AUTO_C_STD)
    if (${AUTO_C_STD_SET})
    else ()
        set(AUTO_C_STD_SET 1)
        set(C_FLAGS "${C_FLAGS} -std=c99")
    endif ()
endif ()

if (AUTO_CXX_STD)
    if (${AUTO_CXX_STD_SET})
    else ()
        set(AUTO_CXX_STD_SET 1)
        set(CXX_FLAGS "${CXX_FLAGS} -std=gnu++0x")
    endif ()
endif ()

# Linker pruning is broken when used with the cog memory model. See the
# following thread for a workaround:
# http://forums.parallax.com/showthread.php/157878-Simple-blinky-program-and-linker-pruning
if (NOT((MODEL STREQUAL "cog") OR (MODEL STREQUAL "COG")))
    if (AUTO_CUT_SECTIONS)
        if (${AUTO_CUT_SECTIONS_SET})
        else ()
            set(AUTO_CUT_SECTIONS_SET 1)
            set(COMMON_FLAGS "${COMMON_FLAGS} -ffunction-sections -fdata-sections")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
        endif ()
    endif ()
endif ()

if (PROPWARE_PRINT_FLOAT)
    if (${PROPWARE_PRINT_FLOAT_SET})
    else ()
        set(PROPWARE_PRINT_FLOAT_SET 1)
        add_definitions(-DENABLE_PROPWARE_PRINT_FLOAT)
    endif ()
endif ()

# Check if a deprecated variable name is set
if (DEFINED CFLAGS OR DEFINED CXXFLAGS)
    if (${CFLAGS_CXXFLAGS_SET})
    else ()
        set(CFLAGS_CXXFLAGS_SET 1)
        message(WARN ": The variables `CFLAGS` and `CXXFLAGS` have been replaced by `C_FLAGS` and `CXX_FLAGS`.")
        set(C_FLAGS ${CFLAGS})
        set(CXX_FLAGS ${CXXFLAGS})
        set(CFLAGS )
        set(CXXFLAGS )
    endif ()
endif()

if (${COMMON_FLAGS_SET})
else ()
    set(COMMON_FLAGS_SET 1)
    set(COMMON_FLAGS "-save-temps ${COMMON_FLAGS}")
    #set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=main.rawmap ${CMAKE_EXE_LINKER_FLAGS}")
endif ()

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
