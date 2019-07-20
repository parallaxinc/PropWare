cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(ArduinoJson_Demo)

create_simple_executable(${PROJECT_NAME} ArduinoJson_Demo.cpp)
