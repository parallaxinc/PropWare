cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(SynchronousPrinter_Demo)

create_simple_executable(${PROJECT_NAME} SynchronousPrinter_Demo.cpp)
