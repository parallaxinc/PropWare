cmake_minimum_required(VERSION 3.3)
find_package(PropWare REQUIRED)

project(PWEdit_Demo)

set(MODEL cmm)
create_simple_executable(${PROJECT_NAME} PWEdit_Demo.cpp)
target_link_libraries(${PROJECT_NAME}
    stdc++)
