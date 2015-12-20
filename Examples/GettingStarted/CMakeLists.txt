cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

set(BOARD QUICKSTART)
set(MODEL lmm)
set(COMMON_FLAGS "-Os")

project(GettingStarted)

create_simple_executable(${PROJECT_NAME} ${PROJECT_NAME})
