cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(Utility_Demo)

create_simple_executable(${PROJECT_NAME} Utility_Demo.cpp)
