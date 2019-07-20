cmake_minimum_required(VERSION 3.12)

# Spin2Cpp is a PropWare 2.1 veature, so you must have at least PropWare 2.1
find_package(PropWare 2.1 REQUIRED)

project(Spin2Cpp_Demo)

# FullDuplexSerial.spin is just a library, so we only pass the name of the Spin file and a name for the output variable
spin2cpp(FullDuplexSerial.spin FullDuplexSerial_CXX_FILES)

# Hello.spin contains our "main" method, so we need to pass a third parameter which marks it as the main file
spin2cpp(Hello.spin Hello_CXX_FILES 1)

# Now we create an executable and include source files that match up to our spin files.
create_simple_executable(Spin2Cpp_Demo ${Hello_CXX_FILES} ${FullDuplexSerial_CXX_FILES})
