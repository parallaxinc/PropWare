cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

project(L3G_Demo)

create_simple_executable(${PROJECT_NAME} L3G_Demo.cpp)
