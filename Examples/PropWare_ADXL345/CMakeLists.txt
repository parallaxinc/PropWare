cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(ADXL345_Demo)

create_simple_executable(${PROJECT_NAME} ADXL345_Demo.cpp)
