cmake_minimum_required(VERSION 3.0)
find_package(PropWare 2.0 REQUIRED)

project(Spin2Dat_Demo CXX SPIN2DAT)

create_simple_executable(${PROJECT_NAME}
    Spin2Dat_Demo
    pst.spin)
