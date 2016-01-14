cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(Hybrid_Demo)

create_simple_executable(${PROJECT_NAME} Hybrid_Demo.cpp)
