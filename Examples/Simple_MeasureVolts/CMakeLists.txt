cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

project(Simple_MeasureVolts)

create_simple_executable(${PROJECT_NAME} MeasureVolts_Demo.c)
