cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

project(DuplexUART_Demo)

create_simple_executable(${PROJECT_NAME} DuplexUART_Demo.cpp)
