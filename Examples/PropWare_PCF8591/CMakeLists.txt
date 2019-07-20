cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(PCF8591_Demo)

create_simple_executable(${PROJECT_NAME} PCF8591_Demo.cpp)
