Creating a PropWare Application   {#projectCreation}
===============================

PropWare makes building and running applications very easy. Below are three different methods for building and
deploying a PropGCC application using PropWare.

Starting a New Project
----------------------
-# The defining piece of a PropWare project is the CMakeLists.txt file. It must be named "CMakeLists.txt". Examples of
   this file can be found in each of the Example projects, but here's the simplest form:
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
   cmake_minimum_required (VERSION 3.0.0)
   # Aside from cmake_minimum_required, this must be the first two lines of the file
   file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)
   set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)

   ##############################################
   # Your code starts here

   project(HelloWorld)

   add_executable(${PROJECT_NAME} main.cpp)

   # Your code ends here
   ##############################################

   # This must be the last line of the file
   include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * Two important lines should be noted: `set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)` and
     `include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)`. These lines load the core of the PropWare build system and
     must exist at the top and bottom respectively of your CMakeLists.txt file _exactly_ as they appear above.
   * Next we name our project and target executable (the elf file): `project(HelloWorld)`. This name can be anything 
     you'd like. For clarity's sake, I've made the project and source file different names, but usually they are the
     same.
   * Finally, we need to tell CMake what files should be compiled: `add_executable(${PROJECT_NAME} main.cpp)`.
     The first parameter _must_ be exactly as you see it - PropWare's build system relies on your executable being named
     the same as your project. The second parameter says that "HelloWorld.elf" should contain the source file 
     "main.cpp". If you have multiple source files in your project, simply append them after main.cpp - each 
     separated by any amount of whitespace.
-# 

Importing PropWare Projects into Eclipse (and other IDEs)
---------------------------------------------------------
-# For Eclipse users, ensure the C/C++ Developer Tools (CDT) plugin is installed; For new Eclipse users, find
   `Eclipse IDE for C/C++ Developers` [here](http://www.eclipse.org/downloads/) and download the appropriate package
-# Coming soon...
