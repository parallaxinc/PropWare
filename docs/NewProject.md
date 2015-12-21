Create a %PropWare Application {#NewProject}
==============================

Starting a New Project
----------------------
1. The defining piece of a %PropWare project is the `CMakeLists.txt` file. It must be named "CMakeLists.txt" as per 
   CMake standards. Full details on CMake files in relation to %PropWare are documented under
   [Build System](@ref BuildSystem), but a typical use case with four source code files is provided below.
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
   cmake_minimum_required (VERSION 3.0.0)
   find_package(PropWare REQUIRED)

   project(Quadcopter C CXX ASM ECOGC)

   create_simple_executable(${PROJECT_NAME} 
       ${PROJECT_NAME}
       motor_drivers
       avionics.S
       rf_transceiver.ecogc)
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
2. Write your source code.
3. At the terminal, enter your project's directory. Type `cmake -G "Unix Makefiles" .`. This will generate a Makefile
   for you.
4. Last step: choose between `make`, `make debug` and `make run`.
  * `make`: Without any arguments, `make` will simply compile your executable - in this case, `HelloWorld.elf`.
  * `make debug`: The `debug` target will load your program into RAM and then start a terminal to monitor and/or
    communicate with the Propeller.
    * Your keyboard's arrow keys are the best way to exit the terminal - press any of them once. Do not use Control-C.
  * `make run`: The `run` target will load your program to EEPROM and return to command prompt.
