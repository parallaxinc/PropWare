Chapter 4: Create a PropWare Application
================================================

[<< Chapter 3: Run an Example Project](http://david.zemon.name/PropWare/md_docs_Ch3RunningExampleProjects.xhtml)<br />
[>> Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml)

%PropWare makes building and running applications easy. Below are instructions for creating a new project, followed by
importing that project into your favorite IDE.

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

Importing %PropWare Projects into Eclipse (and other IDEs)
---------------------------------------------------------
1. For Eclipse users, ensure the C/C++ Developer Tools (CDT) plugin is installed; For new Eclipse users, find
   `Eclipse IDE for C/C++ Developers` [here](http://www.eclipse.org/downloads/) and download the appropriate package
2. Create your [CMakeLists.txt file](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml).
3. For any source file that does not yet exist, create an empty file in its place. CMake will throw errors if your
   CMakeLists.txt file references any non-existing source files.
4. At the terminal, enter your project's directory and type `cmake -G "Eclipse CDT4 - Unix Makefiles" .`. Notice that
   this time, we're adding a hook to tell CMake that we're using Eclipse. If you prefer another IDE, check the [CMake
   Wiki](http://www.cmake.org/Wiki/CMake_Generator_Specific_Information) to see if it is supported.
5. Navigating the Eclipse import dialogue:
  1. Open Eclipse. In the `File` menu, select `Import...`.
  2. Under the `General` group should be an option `Existing Projects into Workspace` - select that option and choose 
     `Next >`.
  3. On the next page, ensure `Select root directory:` is selected at the top and then click `Browse...`. Find the
     parent folder of your project (for example, if your project is `C:\Users\David\Hello`, then choose 
     `C:\Users\David`).
  4. At least one project should populate in the box below. Ensure your new %PropWare project and only your new
     %PropWare project is selected.
  5. Eclipse will import all necessary settings as soon as you click `Finish`. Compile flags, include directories and
     required source files will all be ready to go.
6. Compiling your program and loading to EEPROM can be done easily from within Eclipse. Compiling is as simple as 
   clicking the hammer icon or using `Control-B`. Writing to EEPROM requires creating a new Run Configuration.
  - I do not recommend attempting to create a Run Configuration for %PropWare's `debug` configuration. PropGCC's 
    terminal does not play well with either Eclipse or JetBrains CLion. Use the terminal for debugging, exactly as
    described in "Starting a New Project" step 4.

[<< Chapter 3: Run an Example Project](http://david.zemon.name/PropWare/md_docs_Ch3RunningExampleProjects.xhtml)<br />
[>> Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.xhtml)
