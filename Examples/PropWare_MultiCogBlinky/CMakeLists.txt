cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

set(BOARD QUICKSTART)

project(MultiCogBlinky_Demo)

create_simple_executable(${PROJECT_NAME} MultiCogBlinky_Demo.cpp)
