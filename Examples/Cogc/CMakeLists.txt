cmake_minimum_required(VERSION 3.0)
find_package(PropWare 2.0.0 REQUIRED)

set(MODEL cog)

project(Cogc_Demo CXX COGCXX)

create_simple_executable(${PROJECT_NAME}
    Cogc_Demo
    blinky.cogcpp)
