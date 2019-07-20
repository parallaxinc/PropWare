cmake_minimum_required(VERSION 3.12)
find_package(PropWare REQUIRED)

project(libadcACpropab)

create_simple_executable(${PROJECT_NAME} libadcACpropab.c)
