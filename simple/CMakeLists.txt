include_directories(${PROJECT_SOURCE_DIR})

file(GLOB SIMPLE_OBJECTS
    ${PROJECT_SOURCE_DIR}/simple/*.[!t])

add_subdirectory(cog)
add_subdirectory(cmm)
add_subdirectory(lmm)
add_subdirectory(xmmc)
add_subdirectory(xmm-single)
add_subdirectory(xmm-split)
