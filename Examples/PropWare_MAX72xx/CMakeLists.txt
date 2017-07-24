cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(MAX72xx_Demo)

create_simple_executable(${PROJECT_NAME} MAX72xx_Demo.cpp)
