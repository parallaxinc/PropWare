cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(WS2812_Demo)

create_simple_executable(${PROJECT_NAME} WS2812_Demo.cpp)
