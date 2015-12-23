cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

project(GettingStarted)

create_simple_executable(${PROJECT_NAME} GettingStarted.cpp)
