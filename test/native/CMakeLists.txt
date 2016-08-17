cmake_minimum_required(VERSION 3.0)

project(NativeTests)

include_directories(${CMAKE_SOURCE_DIR}/../..)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)

add_executable(PWEdit_Demo
    PWEdit_Demo.cpp
    mock/filesystem/nativefilereader.h
    mock/filesystem/nativefilewriter.h
    mock/filesystem/nativefilesystem.h
    mock/hmi/input/stdin.h
    mock/hmi/output/stdout.h
    mock/memory/mockstorage.h
    )
target_compile_options(PWEdit_Demo PRIVATE -fpermissive)
