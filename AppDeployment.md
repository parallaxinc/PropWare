Creating a PropWare Application
===============================

PropWare makes building and running applications very easy. Below are three different methods for building and
deploying a PropGCC application using PropWare.

Starting a New Project
----------------------
1. The defining piece of a PropWare project is the `CMakeLists.txt` file. It must be named "CMakeLists.txt" as per CMake
   standards. Examples of this file can be found in each of the Example projects, but here's the simplest form:
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
   #############################################################################
   ### Template code. Do not modify                                            #
                                                                               #
   cmake_minimum_required (VERSION 3.0.0)                                      #
   # Aside from cmake_minimum_required, this must be the first two lines       #
   # of the file                                                               #
   file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                       #
   set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)         #
   #############################################################################

   project(HelloWorld)

   add_executable(${PROJECT_NAME} main.cpp)

   #############################################################################
   ### Template code. Do not modify                                            #
                                                                               #
   # This must be the last line of the file                                    #
   include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)                        #
   #############################################################################
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   * Note the two sections marked as "Template code." Make sure these two pieces begin and end each of your
     CMakeLists.txt files. They load the standard Propeller settings for your project.
   * Next we name our project: `project(HelloWorld)`. This name can be anything you'd like so long as there is no 
     whitespace.
   * Finally, we need to tell CMake what files should be compiled: `add_executable(${PROJECT_NAME} main.cpp)`.
     The first parameter _must_ be exactly as you see it - PropWare's build system relies on your executable being named
     the same as your project. Next, simply list off each of your source files (separated by whitespace).
2. Write your source code.
3. At the terminal, enter your project's directory. Type "`cmake -G "Unix Makefiles" .`". This will generate a Makefile
   for you.
4. Last step: choose between `make`, `make debug` and `make run`.
  * `make`: Without any arguments, `make` will simply compile your executable - in this case, `HelloWorld.elf`.
  * `make debug`: The `debug` target will load your program into RAM and then start a terminal to monitor and/or
    communicate with the Propeller.
    * For those unfamiliar with PropGCC's terminal: Your keyboard's arrow keys are the best way to exit the terminal - 
      press any of them once. Do not use Control-C.
  * `make run`: The `run` target will load your program to EEPROM and return to command prompt.

Importing PropWare Projects into Eclipse (and other IDEs)
---------------------------------------------------------
1. For Eclipse users, ensure the C/C++ Developer Tools (CDT) plugin is installed; For new Eclipse users, find
   `Eclipse IDE for C/C++ Developers` [here](http://www.eclipse.org/downloads/) and download the appropriate package
2. Coming soon...
