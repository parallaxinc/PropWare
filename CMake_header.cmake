#
if (NOT DEFINED PROPGCC_PREFIX)
    set(PROPGCC_PREFIX $ENV{PROPGCC_PREFIX})
    if (NOT PROPGCC_PREFIX)
        message(FATAL_ERROR "Please define 'PROPGCC_PREFIX' either as an environment variable or CMake variable. The value should be the location of PropGCC. For example, on Linux or Mac the default is /opt/parallax.")
    endif (NOT PROPGCC_PREFIX)
endif (NOT DEFINED PROPGCC_PREFIX)

if (NOT DEFINED PROPWARE_PATH)
    set(PROPWARE_PATH $ENV{PROPWARE_PATH})
    if (NOT PROPWARE_PATH)
        message(FATAL_ERROR "Please define 'PROPWARE_PATH' either as an environment variable or CMake variable. The value should be the location of the PropWare root directory.")
    endif (NOT PROPWARE_PATH)
endif (NOT DEFINED PROPWARE_PATH)

if (NOT DEFINED MODEL)
    set(MODEL lmm)
endif (NOT DEFINED MODEL)

if (${MODEL} MATCHES xmm)
    set(MODEL xmm-split)
endif (${MODEL} MATCHES xmm)

if (NOT DEFINED BOARD)
    set(BOARD ${PROPELLER_LOAD_BOARD})
endif (NOT DEFINED BOARD)

if (DEFINED BOARD)
    set(BRDFLAG -b${BOARD})
endif(DEFINED BOARD)

include(CMakeToolchain.cmake)

# Add standard flags
set(ASFLAGS "-m${MODEL} -xassembler-with-cpp")
set(CFLAGS_NO_MODEL "-g -Wall -m32bit-doubles")
set(CFLAGS "${CFLAGS} -m${MODEL} ${CFLAGS_NO_MODEL}")
set(CSTANDARD "-std=c99")
set(CXXFLAGS "${CXXFLAGS} ${CFLAGS} -fno-threadsafe-statics -fno-rtti")
set(LDFLAGS "-Xlinker -Map=main.rawmap")

# Set flags
set(CMAKE_AS_FLAGS ${ASFLAGS})
set(CMAKE_C_FLAGS ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CXXFLAGS})
set(CMAKE_EXE_LINKER_FLAGS ${LDFLAGS})

include_directories(${PROPWARE_PATH})

target_link_libraries(${CMAKE_PROJECT_NAME} PropWare_${MODEL})