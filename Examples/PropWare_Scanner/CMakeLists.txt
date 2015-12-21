cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

project(Scanner_Demo C CXX ASM)

create_simple_executable(${PROJECT_NAME} Scanner_Demo.cpp)
