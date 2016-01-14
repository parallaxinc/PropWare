cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(SimplexUART_Demo C CXX ASM)

create_simple_executable(${PROJECT_NAME} SimplexUART_Demo.cpp)
