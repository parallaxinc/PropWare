cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(MCP3000_Demo)

create_simple_executable(${PROJECT_NAME} MCP3000_Demo.cpp)
