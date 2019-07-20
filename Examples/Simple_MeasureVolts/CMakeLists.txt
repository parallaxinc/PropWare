cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(Simple_MeasureVolts)

create_simple_executable(${PROJECT_NAME} MeasureVolts_Demo.c)
