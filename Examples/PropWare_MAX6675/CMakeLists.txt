cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(MAX6675_Demo)

create_simple_executable(${PROJECT_NAME} MAX6675_Demo.cpp)
