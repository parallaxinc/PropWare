cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(COMMON_FLAGS "-Os")

project(SimplexUART_Demo C CXX ASM)

create_simple_executable(${PROJECT_NAME} ${PROJECT_NAME})
