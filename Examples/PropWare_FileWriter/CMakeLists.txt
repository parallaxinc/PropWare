cmake_minimum_required (VERSION 3.0.0)
find_package(PropWare REQUIRED)

set(BOARD dna)
set(MODEL cmm)

project(FileWriter_Demo)

create_simple_executable(${PROJECT_NAME} ${PROJECT_NAME})
