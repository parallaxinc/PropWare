cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

set(MODEL cog)

project(Blinky_Demo)

create_simple_executable(${PROJECT_NAME} ${PROJECT_NAME})
