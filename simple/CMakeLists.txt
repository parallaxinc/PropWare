file(GLOB SIMPLE_OBJECTS
    ${CMAKE_CURRENT_LIST_DIR}/*.[!t]*)

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
