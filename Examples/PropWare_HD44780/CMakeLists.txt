cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(HD44780_Demo)

create_simple_executable(${PROJECT_NAME} HD44780_Demo.cpp)
