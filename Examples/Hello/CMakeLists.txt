cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

set(LINK_TINY ON)

project(Hello_Demo C CXX SPIN2DAT)

create_simple_executable(${PROJECT_NAME}
    Hello_Demo.cpp
    pst.spin)
