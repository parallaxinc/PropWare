cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

set(BOARD activityboard)

project(FileWriter_Demo)

create_simple_executable(${PROJECT_NAME} FileWriter_Demo.cpp)
