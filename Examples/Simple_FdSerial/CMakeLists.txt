cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(FdSerial_Demo C CXX SPIN2DAT)

create_simple_executable(${PROJECT_NAME} FdSerial_Demo.c pst.spin)
