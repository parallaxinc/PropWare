function(create_pw_lib)
    set(options )
    set(oneValueArgs LIB_NAME MODEL)
    set(multiValueArgs EXTRA_INCLUDE_DIRS SOURCES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TARGET ${ARGS_LIB_NAME}_${ARGS_MODEL})
    list(LENGTH ARGS_SOURCES SRC_COUNT)
    if (SRC_COUNT)
        add_library(${TARGET} ${ARGS_SOURCES})
    endif ()
    set_compile_flags(${TARGET} ${ARGS_MODEL})
    if (NOT LIB_NAME STREQUAL "PropWare")
        target_compile_options(${TARGET} PRIVATE -w)
        if (LIB_NAME STREQUAL "Simple")
            set(SYSTEM_ARG SYSTEM)
        endif ()
    endif ()
    foreach(d IN LISTS ARGS_EXTRA_INCLUDE_DIRS)
        list(APPEND BUILD_INTERFACE_DIRS $<BUILD_INTERFACE:${d}>)
    endforeach()
    target_include_directories(${TARGET} ${SYSTEM_ARG}
        PUBLIC
            ${BUILD_INTERFACE_DIRS}
            $<INSTALL_INTERFACE:share/PropWare/include/${ARGS_LIB_NAME}>
    )
    set_linker(${TARGET})
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${ARGS_LIB_NAME})
    install(TARGETS ${TARGET}
        DESTINATION share/PropWare/lib/${ARGS_MODEL}
        COMPONENT propware
        EXPORT PropWare-targets)
endfunction()
add_subdirectory(external_libs)
add_subdirectory(PropWare)
add_subdirectory(Examples)
add_subdirectory(test)
