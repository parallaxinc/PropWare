cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(BufferedUARTRX_Demo CXX COGCXX)

create_simple_executable(${PROJECT_NAME}
    BufferedUART_Demo.cpp
    buffereduarttx.cogcpp
    buffereduartrx.cogcpp
)
