Chapter 4: Create a PropWare Application
================================================

[<< Chapter 3: Run an Example Project](http://david.zemon.name/PropWare/md_docs_Ch3RunningExampleProjects.xhtml)<br />
[>> Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml)

Starting a New Project
----------------------
1. The defining piece of a %PropWare project is the `CMakeLists.txt` file. It must be named "CMakeLists.txt" as per CMake
   standards. Full details on CMake files in relation to %PropWare can be found in
   [Appendix A](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml), but a typical use case with four 
   source code files is provided below.
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
   ####################################################################################
   ### Template code. Do not modify                                                   #
                                                                                      #
   cmake_minimum_required (VERSION 3.0.0)                                             #
   # Aside from cmake_minimum_required, this must be the first two lines of the file  #
   file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                              #
   include(${PROPWARE_PATH}/CMakePropellerHeader.cmake)                               #
   ####################################################################################

   set(BOARD QUICKSTART)
   set(MODEL cmm)
   set(COMMON_FLAGS "-Os")
   set(ECOGC_FLAGS "--bogus")
   
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
    * For those unfamiliar with PropGCC's terminal: Your keyboard's arrow keys are the best way to exit the terminal - 
      press any of them once. Do not use Control-C.
  * `make run`: The `run` target will load your program to EEPROM and return to command prompt.

[<< Chapter 3: Run an Example Project](http://david.zemon.name/PropWare/md_docs_Ch3RunningExampleProjects.xhtml)<br />
[>> Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml)
