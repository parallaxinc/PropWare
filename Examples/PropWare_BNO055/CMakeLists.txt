cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(BNO055_Demo)

create_simple_executable(${PROJECT_NAME} BNO055_Demo.cpp)
